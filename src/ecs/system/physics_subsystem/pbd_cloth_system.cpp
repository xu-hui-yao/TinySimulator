#include <ecs/component/cloth.h>
#include <ecs/component/transform.h>
#include <ecs/system/physics_subsystem/pbd_cloth_system.h>

int PBDClothSystem::execution_priority() const { return priority; }

void PBDClothSystem::update(entt::registry &registry, float dt) {
    auto view           = registry.view<Cloth, Transform>();
    auto collision_view = registry.view<Collider, Transform>();
    view.each([&](Cloth &cloth, const Transform &transform) {
        // Phase 1: Predict positions with external forces
        predict_positions(cloth, dt);
        // Phase 2: Handle collisions
        collision_view.each([&](const Collider &collider, const Transform &collider_transform) {
            handle_collisions(cloth, collider, collider_transform);
        });
        // Phase 3: Solve constraints iteratively
        solve_constraints(cloth);
        // Phase 4: Update positions and velocities
        update_positions(cloth, dt);
        // Update model to render
        cloth.update_model(transform);
    });
}

void PBDClothSystem::predict_positions(Cloth &cloth, float dt) {
    for (size_t i = 0; i < cloth.positions.size(); ++i) {
        glm::vec3 acceleration = cloth.field_force * cloth.inv_masses[i] + cloth.gravity;
        cloth.velocities[i] += acceleration * dt;
        cloth.velocities[i] *= std::max(1.0f - cloth.damping * cloth.inv_masses[i] * dt, 0.0f);
        cloth.pred_positions[i] = cloth.positions[i] + cloth.velocities[i] * dt;
    }
}

void PBDClothSystem::handle_collisions(Cloth &cloth, const Collider &collider, const Transform &collider_transform) {
    // Early exit if collider is inactive
    if (!collider.is_active)
        return;
    // Cache collider type to avoid repeated access
    const Collider::ShapeType collider_type = collider.shape;
    // Process each cloth vertex
    for (size_t i = 0; i < cloth.positions.size(); ++i) {
        // Skip fixed vertices and infinite mass particles
        if (cloth.fixed_vertices[i] || cloth.inv_masses[i] <= 0.0f)
            continue;
        // Get predicted position (current simulation state)
        glm::vec3 &pred_pos = cloth.pred_positions[i];
        glm::vec3 surface_pos, normal;
        bool collision = false;
        // Dispatch collision check based on collider type
        switch (collider_type) {
            case Collider::SPHERE:
                collision = collide_sphere(pred_pos, collider, collider_transform, surface_pos, normal);
                break;
            case Collider::BOX:
                collision = collide_box(pred_pos, collider, collider_transform, surface_pos, normal);
                break;
            case Collider::CAPSULE:
                collision = collide_capsule(pred_pos, collider, collider_transform, surface_pos, normal);
                break;
            default:
                get_logger()->error("Collider type not recognized");
                break;
        }
        if (collision) {
            constexpr float epsilon = 1e-3f;
            // Apply position correction (project out of collider)
            pred_pos = surface_pos + normal * epsilon;
            // Apply velocity damping in normal direction
            const float velocity_normal = glm::dot(cloth.velocities[i], normal);
            if (velocity_normal < 0) {
                // Remove velocity component going into the collider
                cloth.velocities[i] -= velocity_normal * normal;
            }
            // Simple friction approximation (tangential velocity reduction)
            const glm::vec3 tangent_vel = cloth.velocities[i] - velocity_normal * normal;
            cloth.velocities[i] -= tangent_vel * (1.0f - cloth.friction_factor);
        }
    }
}

void PBDClothSystem::solve_constraints(Cloth &cloth) {
    for (auto &constraint : cloth.constraints) {
        constraint->project(cloth, solver_iterations);
    }
}

void PBDClothSystem::update_positions(Cloth &cloth, float dt) {
    for (size_t i = 0; i < cloth.positions.size(); ++i) {
        if (!cloth.fixed_vertices[i]) {
            constexpr float epsilon = 1e-4f;
            cloth.velocities[i]     = (cloth.pred_positions[i] - cloth.positions[i]) / std::max(dt, epsilon);
            cloth.positions[i]      = cloth.pred_positions[i];
        }
    }
}

/**
 * @brief Checks collision between a point and sphere collider
 * @param point World space point to test
 * @param collider Sphere collider configuration
 * @param transform Collider's transform
 * @param[out] surface_pos Closest point on collider surface
 * @param[out] normal Surface normal at collision point
 * @return True if collision occurs
 */
bool PBDClothSystem::collide_sphere(const glm::vec3 &point, const Collider &collider, const Transform &transform,
                                    glm::vec3 &surface_pos, glm::vec3 &normal) {
    // Calculate world space collider center
    const glm::vec3 center = transform.position + collider.offset;
    const float radius     = collider.radius;
    const glm::vec3 delta  = point - center;
    const float dist       = glm::length(delta);
    if (dist < radius) {
        // Handle degenerate case where point is exactly at center
        if (dist < 1e-6f) {
            surface_pos = center + glm::vec3(radius, 0, 0);
            normal      = glm::vec3(1, 0, 0);
        } else {
            surface_pos = center + delta * (radius / dist);
            normal      = delta / dist;
        }
        return true;
    }
    return false;
}

/**
 * @brief Checks collision between a point and box collider
 * @param point World space point to test
 * @param collider Box collider configuration
 * @param transform Collider's transform
 * @param[out] surface_pos Closest point on collider surface
 * @param[out] normal Surface normal at collision point
 * @return True if collision occurs
 */
bool PBDClothSystem::collide_box(const glm::vec3 &point, const Collider &collider, const Transform &transform,
                                 glm::vec3 &surface_pos, glm::vec3 &normal) {
    // Convert point to collider's local space
    const glm::mat4 world_to_local =
        glm::inverse(transform.matrix() * glm::translate(glm::mat4(1.0f), collider.offset));
    const glm::vec3 local_point = glm::vec3(world_to_local * glm::vec4(point, 1.0f));

    const glm::vec3 half = collider.half_extents;

    // Check bounding box collision
    if (local_point.x > -half.x && local_point.x < half.x && local_point.y > -half.y && local_point.y < half.y &&
        local_point.z > -half.z && local_point.z < half.z) {

        // Find closest axis
        glm::vec3 penetration = half - glm::abs(local_point);
        int closest_axis      = 0;
        float min_penetration = penetration.x;
        if (penetration.y < min_penetration) {
            min_penetration = penetration.y;
            closest_axis    = 1;
        }
        if (penetration.z < min_penetration) {
            closest_axis = 2;
        }
        // Calculate local space surface point and normal
        glm::vec3 local_normal(0.0f);
        glm::vec3 local_surface = local_point;

        if (local_point[closest_axis] > 0) {
            local_surface[closest_axis] = half[closest_axis];
            local_normal[closest_axis]  = 1.0f;
        } else {
            local_surface[closest_axis] = -half[closest_axis];
            local_normal[closest_axis]  = -1.0f;
        }
        // Convert back to world space
        const glm::mat4 local_to_world = transform.matrix() * glm::translate(glm::mat4(1.0f), collider.offset);
        surface_pos                    = glm::vec3(local_to_world * glm::vec4(local_surface, 1.0f));
        normal                         = glm::normalize(glm::vec3(local_to_world * glm::vec4(local_normal, 0.0f)));

        return true;
    }
    return false;
}
/**
 * @brief Checks collision between a point and capsule collider
 * @param point World space point to test
 * @param collider Capsule collider configuration
 * @param transform Collider's transform
 * @param[out] surface_pos Closest point on collider surface
 * @param[out] normal Surface normal at collision point
 * @return True if collision occurs
 */
bool PBDClothSystem::collide_capsule(const glm::vec3 &point, const Collider &collider, const Transform &transform,
                                     glm::vec3 &surface_pos, glm::vec3 &normal) {
    // Convert point to collider's local space
    const glm::mat4 world_to_local =
        glm::inverse(transform.matrix() * glm::translate(glm::mat4(1.0f), collider.offset));
    const glm::vec3 local_point = glm::vec3(world_to_local * glm::vec4(point, 1.0f));
    // Capsule parameters
    const float radius      = collider.capsule_radius;
    const float half_height = (collider.capsule_height - collider.capsule_radius * 2) * 0.5f;
    const glm::vec3 a(0.0f, -half_height, 0.0f); // Bottom hemisphere
    const glm::vec3 b(0.0f, half_height, 0.0f);  // Top hemisphere
    // Find the closest point on capsule segment
    const glm::vec3 ab                    = b - a;
    const float t                         = glm::clamp(glm::dot(local_point - a, ab) / glm::dot(ab, ab), 0.0f, 1.0f);
    const glm::vec3 closest_segment_point = a + t * ab;
    // Calculate penetration
    const glm::vec3 delta = local_point - closest_segment_point;
    const float dist      = glm::length(delta);
    if (dist < radius) {
        // Calculate surface point in local space
        glm::vec3 local_surface = closest_segment_point;
        if (dist > 1e-6f) {
            local_surface += delta * (radius / dist);
        } else {
            // Handle degenerate case
            local_surface += glm::vec3(radius, 0, 0);
        }
        // Calculate normal in local space
        glm::vec3 local_normal = delta;
        if (dist > 1e-6f) {
            local_normal /= dist;
        } else {
            local_normal = glm::vec3(1, 0, 0);
        }
        // Convert results back to world space
        const glm::mat4 local_to_world = transform.matrix() * glm::translate(glm::mat4(1.0f), collider.offset);
        surface_pos                    = glm::vec3(local_to_world * glm::vec4(local_surface, 1.0f));
        normal                         = glm::normalize(glm::vec3(local_to_world * glm::vec4(local_normal, 0.0f)));
        return true;
    }
    return false;
}