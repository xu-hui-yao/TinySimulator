#include <ecs/component/rigidbody.h>
#include <ecs/system/physics.h>
#include <glm/gtx/quaternion.hpp>

std::vector<PhysicsSystem::CollisionPair> PhysicsSystem::collision_pairs;

void PhysicsSystem::update(entt::registry &registry, float dt) {
    integrate_forces(registry, dt);
    detect_collisions(registry);
    resolve_collisions(registry, dt);
}

void PhysicsSystem::integrate_forces(entt::registry &registry, float dt) {
    auto view = registry.view<Transform, RigidBody>();
    view.each([dt](auto &t, auto &rb) {
        if (rb.is_kinematic || rb.mass <= 0.0f)
            return;
        rb.integrate(dt, gravity);
        t.position += rb.linear_velocity * dt;
        // Quaternion integration with proper angular velocity handling
        glm::quat rotation = t.orientation();
        const glm::quat angular_quat(0, rb.angular_velocity.x, rb.angular_velocity.y, rb.angular_velocity.z);
        rotation   = glm::normalize(rotation + 0.5f * dt * angular_quat * rotation);
        t.rotation = glm::eulerAngles(rotation);
    });
}

void PhysicsSystem::detect_collisions(entt::registry &registry) {
    collision_pairs.clear();
    auto view = registry.view<Transform, Collider>();
    std::vector<entt::entity> entities;
    view.each([&](auto entity, auto &&...) { entities.push_back(entity); });
    // Optimized pair checking with early rejection
    for (size_t i = 0; i < entities.size(); ++i) {
        for (size_t j = i + 1; j < entities.size(); ++j) {
            const auto ent_a    = entities[i];
            const auto ent_b    = entities[j];
            const auto &trans_a = view.get<Transform>(ent_a);
            const auto &coll_a  = view.get<Collider>(ent_a);
            const auto &trans_b = view.get<Transform>(ent_b);
            const auto &coll_b  = view.get<Collider>(ent_b);
            if (!coll_a.is_active || !coll_b.is_active)
                continue;
            Collider::CollisionManifold manifold{};
            if (Collider::collide(trans_a, coll_a, trans_b, coll_b, manifold)) {
                collision_pairs.push_back({ ent_a, ent_b, manifold });
            }
        }
    }
}

void PhysicsSystem::resolve_collisions(entt::registry &registry, float dt) {
    for (auto &pair : collision_pairs) {
        auto [ent_a, ent_b, manifold] = pair;
        auto *rb_a                    = registry.try_get<RigidBody>(ent_a);
        auto *rb_b                    = registry.try_get<RigidBody>(ent_b);
        auto *trans_a                 = registry.try_get<Transform>(ent_a);
        auto *trans_b                 = registry.try_get<Transform>(ent_b);
        // Skip trigger interactions
        if (registry.get<Collider>(ent_a).is_trigger || registry.get<Collider>(ent_b).is_trigger)
            continue;
        // Calculate relative positions
        const glm::vec3 r_a = manifold.contact_point - trans_a->position;
        const glm::vec3 r_b = manifold.contact_point - trans_b->position;
        // Calculate velocities with proper null checks
        const glm::vec3 vel_a   = rb_a ? rb_a->linear_velocity + glm::cross(rb_a->angular_velocity, r_a) : glm::vec3(0);
        const glm::vec3 vel_b   = rb_b ? rb_b->linear_velocity + glm::cross(rb_b->angular_velocity, r_b) : glm::vec3(0);
        const glm::vec3 rel_vel = vel_a - vel_b;
        // Normal impulse calculation
        const float restitution = std::min(manifold.combined_restitution, 1.0f);
        float numerator         = -(1.0f + restitution) * glm::dot(rel_vel, manifold.normal);
        // Handle infinite masses safely
        const float inv_mass_a    = (rb_a && rb_a->mass > 0.0f) ? 1.0f / rb_a->mass : 0.0f;
        const float inv_mass_b    = (rb_b && rb_b->mass > 0.0f) ? 1.0f / rb_b->mass : 0.0f;
        const glm::vec3 cross_a   = glm::cross(r_a, manifold.normal);
        const glm::vec3 cross_b   = glm::cross(r_b, manifold.normal);
        const float inv_inertia_a = rb_a ? glm::dot(cross_a, rb_a->inv_inertia_tensor * cross_a) : 0.0f;
        const float inv_inertia_b = rb_b ? glm::dot(cross_b, rb_b->inv_inertia_tensor * cross_b) : 0.0f;
        float denominator         = inv_mass_a + inv_mass_b + inv_inertia_a + inv_inertia_b;
        if (denominator <= 0.0f)
            continue;
        const float impulse_magnitude = numerator / denominator;
        const glm::vec3 impulse       = impulse_magnitude * manifold.normal;
        // Apply impulses with mass validation
        if (rb_a && rb_a->mass > 0.0f) {
            rb_a->linear_velocity += impulse * inv_mass_a;
            rb_a->angular_velocity += rb_a->inv_inertia_tensor * glm::cross(r_a, impulse);
        }
        if (rb_b && rb_b->mass > 0.0f) {
            rb_b->linear_velocity -= impulse * inv_mass_b;
            rb_b->angular_velocity -= rb_b->inv_inertia_tensor * glm::cross(r_b, impulse);
        }
        // Friction handling
        glm::vec3 tangent = rel_vel - glm::dot(rel_vel, manifold.normal) * manifold.normal;
        if (glm::length(tangent) > 1e-6f) {
            tangent   = glm::normalize(tangent);
            numerator = -glm::dot(rel_vel, tangent);
            denominator =
                inv_mass_a + inv_mass_b +
                glm::dot(glm::cross(rb_a ? rb_a->inv_inertia_tensor * glm::cross(r_a, tangent) : glm::vec3(0), r_a),
                         tangent) +
                glm::dot(glm::cross(rb_b ? rb_b->inv_inertia_tensor * glm::cross(r_b, tangent) : glm::vec3(0), r_b),
                         tangent);

            if (denominator <= 0)
                continue;

            float jt = numerator / denominator;
            float mu = manifold.combined_friction;
            jt       = glm::clamp(jt, -impulse_magnitude * mu, impulse_magnitude * mu);

            glm::vec3 frictionImpulse = jt * tangent;

            if (rb_a) {
                rb_a->linear_velocity += frictionImpulse * inv_mass_a;
                rb_a->angular_velocity += rb_a->inv_inertia_tensor * glm::cross(r_a, frictionImpulse);
            }
            if (rb_b) {
                rb_b->linear_velocity -= frictionImpulse * inv_mass_b;
                rb_b->angular_velocity -= rb_b->inv_inertia_tensor * glm::cross(r_b, frictionImpulse);
            }
        }
        // Position correction with safe division
        constexpr float slop              = 0.01f;
        constexpr float correction_factor = 0.8f;
        const float total_inv_mass        = inv_mass_a + inv_mass_b;
        if (total_inv_mass <= 0.0f)
            continue;
        const glm::vec3 correction =
            correction_factor * std::max(manifold.penetration_depth - slop, 0.0f) / total_inv_mass * manifold.normal;
        if (rb_a && rb_a->mass > 0.0f)
            trans_a->position -= correction * inv_mass_a;
        if (rb_b && rb_b->mass > 0.0f)
            trans_b->position += correction * inv_mass_b;
    }
}
