#include <ecs/component/collider.h>
#include <ecs/component/rigidbody.h>
#include <ecs/component/transform.h>
#include <ecs/system/physics_subsystem/collision_system.h>
#include <glm/gtc/quaternion.hpp>

int CollisionSystem::execution_priority() const { return priority; }

void CollisionSystem::update(entt::registry &registry, float dt) {
    detect_collisions(registry);
    resolve_collisions(registry, dt);
}

void CollisionSystem::detect_collisions(entt::registry &registry) {
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
            CollisionManifold manifold{};
            if (collide(trans_a, coll_a, trans_b, coll_b, manifold)) {
                collision_pairs.push_back({ ent_a, ent_b, manifold });
            }
        }
    }
}

void CollisionSystem::resolve_collisions(entt::registry &registry, float dt) {
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

bool CollisionSystem::collide(const Transform &a_t, const Collider &a_c, const Transform &b_t, const Collider &b_c,
                              CollisionManifold &out) {
    const auto shape1 = a_c.shape;
    const auto shape2 = b_c.shape;
    // Handle shape order permutations
    if (shape1 == Collider::SPHERE) {
        if (shape2 == Collider::SPHERE)
            return sphere_vs_sphere(a_t, a_c, b_t, b_c, out);
        if (shape2 == Collider::BOX)
            return sphere_vs_box(a_t, a_c, b_t, b_c, out);
        if (shape2 == Collider::CAPSULE)
            return sphere_vs_capsule(a_t, a_c, b_t, b_c, out);
    } else if (shape1 == Collider::BOX) {
        if (shape2 == Collider::SPHERE)
            return sphere_vs_box(b_t, b_c, a_t, a_c, out);
        if (shape2 == Collider::BOX)
            return box_vs_box(a_t, a_c, b_t, b_c, out);
        if (shape2 == Collider::CAPSULE)
            return box_vs_capsule(a_t, a_c, b_t, b_c, out);
    } else if (shape1 == Collider::CAPSULE) {
        if (shape2 == Collider::SPHERE)
            return sphere_vs_capsule(b_t, b_c, a_t, a_c, out);
        if (shape2 == Collider::BOX)
            return box_vs_capsule(b_t, b_c, a_t, a_c, out);
        if (shape2 == Collider::CAPSULE)
            return capsule_vs_capsule(a_t, a_c, b_t, b_c, out);
    }
    return false;
}

bool CollisionSystem::sphere_vs_sphere(const Transform &a_t, const Collider &a_c, const Transform &b_t,
                                       const Collider &b_c, CollisionManifold &out) {
    const glm::vec3 center_a    = a_t.position + a_c.offset;
    const glm::vec3 center_b    = b_t.position + b_c.offset;
    const glm::vec3 delta       = center_b - center_a;
    const float distance        = glm::length(delta);
    const float combined_radius = a_c.radius + b_c.radius;
    if (distance < combined_radius) {
        out.normal               = glm::normalize(delta);
        out.contact_point        = center_a + out.normal * a_c.radius;
        out.penetration_depth    = combined_radius - distance;
        out.combined_friction    = std::sqrt(a_c.friction * b_c.friction);
        out.combined_restitution = std::sqrt(a_c.restitution * b_c.restitution);
        return true;
    }
    return false;
}

bool CollisionSystem::sphere_vs_box(const Transform &sphere_t, const Collider &sphere_c, const Transform &box_t,
                                    const Collider &box_c, CollisionManifold &out) {
    // Transform sphere center to box's local space (including rotation and scaling)
    const glm::mat4 inv_box       = glm::inverse(box_t.matrix());
    const glm::vec3 sphere_world  = sphere_t.position + sphere_c.offset;
    const glm::vec3 sphere_center = glm::vec3(inv_box * glm::vec4(sphere_world, 1.0f));
    // Calculate the closest point in box's local space (axis-aligned clamp)
    const glm::vec3 closest_local = glm::clamp(sphere_center,
                                               -box_c.half_extents, // Should already include scale factors
                                               box_c.half_extents);
    // Convert the closest point back to world space
    const glm::vec3 closest_world = box_t.matrix() * glm::vec4(closest_local, 1.0f);
    const glm::vec3 delta         = sphere_world - closest_world;
    const float distance          = glm::length(delta);
    const float radius            = sphere_c.radius;
    constexpr float epsilon       = 1e-6f;
    if (distance < radius) {
        // Handle near-zero distance case (sphere center inside box)
        if (distance < epsilon) {
            // Find penetration direction in local space
            glm::vec3 box_local_normal(0.0f);
            const glm::vec3 penetration = box_c.half_extents - glm::abs(sphere_center);

            // Select axis with minimal penetration
            if (penetration.x < penetration.y && penetration.x < penetration.z) {
                box_local_normal.x = (sphere_center.x > 0) ? 1.0f : -1.0f;
            } else if (penetration.y < penetration.z) {
                box_local_normal.y = (sphere_center.y > 0) ? 1.0f : -1.0f;
            } else {
                box_local_normal.z = (sphere_center.z > 0) ? 1.0f : -1.0f;
            }
            out.normal            = glm::normalize(box_t.matrix() * glm::vec4(box_local_normal, 0.0f));
            out.penetration_depth = radius + glm::length(closest_world - sphere_world);
        } else {
            out.normal            = glm::normalize(delta);
            out.penetration_depth = radius - distance;
        }
        // Calculate contact point on sphere surface
        out.contact_point = closest_world + out.normal * sphere_c.radius;
        // Combine material properties
        out.combined_friction    = std::sqrt(sphere_c.friction * box_c.friction);
        out.combined_restitution = std::sqrt(sphere_c.restitution * box_c.restitution);
        return true;
    }
    return false;
}

bool CollisionSystem::sphere_vs_capsule(const Transform &sphere_t, const Collider &sphere_c, const Transform &capsule_t,
                                        const Collider &capsule_c, CollisionManifold &out) {
    // Get capsule endpoints considering rotation and proper height calculation
    glm::vec3 capsule_base, capsule_top;
    get_capsule_endpoints(capsule_t, capsule_c, capsule_base, capsule_top);

    // Sphere center in world space
    const glm::vec3 sphere_center = sphere_t.position + sphere_c.offset;

    // Find closest point on capsule's central segment
    const glm::vec3 closest     = closest_point_on_line_segment(sphere_center, capsule_base, capsule_top);
    const glm::vec3 delta       = sphere_center - closest;
    const float distance        = glm::length(delta);
    const float combined_radius = sphere_c.radius + capsule_c.capsule_radius;
    constexpr float epsilon     = 1e-6f;
    if (distance < combined_radius - epsilon) {
        // Handle near-zero distance cases (sphere center at capsule surface)
        if (distance < epsilon) {
            glm::vec3 segment_dir = capsule_top - capsule_base;
            float seg_length      = glm::length(segment_dir);

            // Degenerate capsule (treat as sphere-to-sphere collision)
            if (seg_length < epsilon) {
                out.normal = glm::normalize(sphere_center - capsule_base);
            }
            // Valid capsule segment
            else {
                segment_dir /= seg_length;
                glm::vec3 to_sphere = sphere_center - capsule_base;

                // Calculate perpendicular component
                float projection        = glm::dot(to_sphere, segment_dir);
                glm::vec3 perpendicular = to_sphere - projection * segment_dir;
                float perp_len          = glm::length(perpendicular);

                if (perp_len > epsilon) {
                    out.normal = glm::normalize(perpendicular);
                } else {
                    // Sphere center on the segment axis, generate orthogonal normal
                    glm::vec3 arbitrary = std::abs(segment_dir.x) > 0.1f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
                    out.normal          = glm::normalize(glm::cross(segment_dir, arbitrary));
                }
            }
        } else {
            out.normal = glm::normalize(delta);
        }
        // Calculate contact point on capsule surface
        out.contact_point     = closest + out.normal * capsule_c.capsule_radius;
        out.penetration_depth = combined_radius - distance;

        // Combine physical properties
        out.combined_friction    = std::sqrt(sphere_c.friction * capsule_c.friction);
        out.combined_restitution = std::sqrt(sphere_c.restitution * capsule_c.restitution);
        return true;
    }
    return false;
}

bool CollisionSystem::box_vs_capsule(const Transform &box_t, const Collider &box_c, const Transform &capsule_t,
                                     const Collider &capsule_c, CollisionManifold &out) {
    // Calculate capsule endpoints with rotation
    glm::vec3 capsule_base, capsule_top;
    get_capsule_endpoints(capsule_t, capsule_c, capsule_base, capsule_top);
    // Convert to box's local space
    const glm::mat4 inv_box    = glm::inverse(box_t.matrix());
    const glm::vec3 local_base = glm::vec3(inv_box * glm::vec4(capsule_base, 1.0f));
    const glm::vec3 local_top  = glm::vec3(inv_box * glm::vec4(capsule_top, 1.0f));
    // Find the closest point in box space
    const glm::vec3 local_closest = closest_point_on_line_segment(glm::vec3(0), local_base, local_top);
    const glm::vec3 box_closest   = glm::clamp(local_closest, -box_c.half_extents, box_c.half_extents);
    // Convert back to world space
    const glm::vec3 world_box_closest     = box_t.matrix() * glm::vec4(box_closest, 1.0f);
    const glm::vec3 world_capsule_closest = box_t.matrix() * glm::vec4(local_closest, 1.0f);
    const glm::vec3 delta                 = world_capsule_closest - world_box_closest;
    const float dist                      = glm::length(delta);

    if (dist < capsule_c.capsule_radius) {
        out.normal               = glm::normalize(delta);
        out.contact_point        = world_box_closest + out.normal * capsule_c.capsule_radius;
        out.penetration_depth    = capsule_c.capsule_radius - dist;
        out.combined_friction    = std::sqrt(box_c.friction * capsule_c.friction);
        out.combined_restitution = std::sqrt(box_c.restitution * capsule_c.restitution);
        return true;
    }
    return false;
}

bool CollisionSystem::capsule_vs_capsule(const Transform &a_t, const Collider &a_c, const Transform &b_t,
                                         const Collider &b_c, CollisionManifold &out) {
    // 胶囊A的端点
    glm::vec3 a_base = a_t.position + a_c.offset;
    glm::vec3 a_top  = a_base;
    a_top.y += a_c.capsule_height;

    // 胶囊B的端点
    glm::vec3 b_base = b_t.position + b_c.offset;
    glm::vec3 b_top  = b_base;
    b_top.y += b_c.capsule_height;

    // 计算最近点对
    auto [closest_a, closest_b] = closest_points_between_lines(a_base, a_top, b_base, b_top);

    glm::vec3 delta       = closest_b - closest_a;
    float dist            = glm::length(delta);
    float combined_radius = a_c.capsule_radius + b_c.capsule_radius;

    if (dist < combined_radius) {
        out.normal               = glm::normalize(delta);
        out.contact_point        = (closest_a + closest_b) * 0.5f;
        out.penetration_depth    = combined_radius - dist;
        out.combined_friction    = std::sqrt(a_c.friction * b_c.friction);
        out.combined_restitution = std::sqrt(a_c.restitution * b_c.restitution);
        return true;
    }
    return false;
}

bool CollisionSystem::box_vs_box(const Transform &a_t, const Collider &a_c, const Transform &b_t, const Collider &b_c,
                                 CollisionManifold &out) {
    // Transform to A's local space
    glm::mat4 a_inv  = glm::inverse(a_t.matrix());
    glm::mat4 b_to_a = a_inv * b_t.matrix();
    glm::mat3 b_rot(b_to_a);

    // Get box centers and extents
    glm::vec3 a_center  = a_c.offset;
    auto b_center       = glm::vec3(b_to_a[3]);
    glm::vec3 a_extents = a_c.half_extents;
    glm::vec3 b_extents = b_c.half_extents;

    float penetration = FLT_MAX;
    glm::vec3 best_normal;
    glm::vec3 delta = b_center - a_center;

    // Test A's local axes
    for (int i = 0; i < 3; ++i) {
        // Calculate B's projection radius on A's axis
        float b_proj = b_extents.x * std::abs(b_rot[0][i]) + b_extents.y * std::abs(b_rot[1][i]) +
                       b_extents.z * std::abs(b_rot[2][i]);

        float a_proj   = a_extents[i];
        float distance = std::abs(delta[i]);
        float overlap  = a_proj + b_proj - distance;

        if (overlap < 0)
            return false;
        if (overlap < penetration) {
            penetration    = overlap;
            best_normal    = glm::vec3(0);
            best_normal[i] = delta[i] > 0 ? 1 : -1;
        }
    }

    // Test B's local axes
    for (int i = 0; i < 3; ++i) {
        glm::vec3 axis = glm::normalize(glm::vec3(b_rot[i]));
        float proj_a = a_extents.x * std::abs(axis.x) + a_extents.y * std::abs(axis.y) + a_extents.z * std::abs(axis.z);
        float proj_b = b_extents[i];
        float distance = std::abs(glm::dot(delta, axis));
        float overlap  = proj_a + proj_b - distance;

        if (overlap < 0)
            return false;
        if (overlap < penetration) {
            penetration = overlap;
            best_normal = axis * (glm::dot(delta, axis) > 0 ? 1.0f : -1.0f);
        }
    }

    // Test edge cross products
    constexpr float epsilon = 1e-6f;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            glm::vec3 axis_a(0);
            axis_a[i]   = 1.0f;
            auto axis_b = glm::vec3(b_rot[j]);

            glm::vec3 cross_axis = glm::cross(axis_a, axis_b);
            float length         = glm::length(cross_axis);
            if (length < epsilon)
                continue;

            cross_axis /= length;
            float a_proj = a_extents.x * std::abs(cross_axis.x) + a_extents.y * std::abs(cross_axis.y) +
                           a_extents.z * std::abs(cross_axis.z);

            float b_proj = b_extents.x * std::abs(glm::dot(b_rot[0], cross_axis)) +
                           b_extents.y * std::abs(glm::dot(b_rot[1], cross_axis)) +
                           b_extents.z * std::abs(glm::dot(b_rot[2], cross_axis));

            float distance = std::abs(glm::dot(delta, cross_axis));
            float overlap  = a_proj + b_proj - distance;

            if (overlap < 0)
                return false;
            if (overlap < penetration) {
                penetration = overlap;
                best_normal = cross_axis * (glm::dot(delta, cross_axis) > 0 ? 1.0f : -1.0f);
            }
        }
    }

    // Calculate final collision data
    if (penetration < FLT_MAX) {
        // Transform normal to world space
        best_normal = glm::normalize(glm::vec3(a_t.matrix() * glm::vec4(best_normal, 0.0f)));

        // Calculate contact point (improved approximation)
        glm::vec3 a_world_center = a_t.position + a_c.offset;
        glm::vec3 b_world_center = b_t.position + b_c.offset;
        out.contact_point        = a_world_center + (b_world_center - a_world_center) * 0.5f;

        out.normal               = best_normal;
        out.penetration_depth    = penetration;
        out.combined_friction    = std::sqrt(a_c.friction * b_c.friction);
        out.combined_restitution = std::sqrt(a_c.restitution * b_c.restitution);
        return true;
    }

    return false;
}

glm::vec3 CollisionSystem::closest_point_on_line_segment(const glm::vec3 &point, const glm::vec3 &a,
                                                         const glm::vec3 &b) {
    glm::vec3 ab = b - a;
    float t      = glm::dot(point - a, ab) / glm::dot(ab, ab);
    t            = glm::clamp(t, 0.0f, 1.0f);
    return a + t * ab;
}

std::pair<float, float> CollisionSystem::find_min(const glm::vec3 &point, const glm::vec3 &seg_start,
                                                  const glm::vec3 &seg_end) {
    // Calculate the closest point on segment to the given point
    const glm::vec3 dir   = seg_end - seg_start;
    const float length_sq = glm::dot(dir, dir);

    // Handle degenerate segment (start == end)
    if (length_sq < 1e-6f) {
        return { 0.0f, glm::distance(point, seg_start) };
    }

    // Project point onto the line and clamp to segment
    const float t         = glm::dot(point - seg_start, dir) / length_sq;
    const float clamped_t = glm::clamp(t, 0.0f, 1.0f);
    const glm::vec3 proj  = seg_start + clamped_t * dir;

    return { clamped_t, glm::distance(point, proj) };
}

std::pair<glm::vec3, glm::vec3> CollisionSystem::closest_points_between_lines(const glm::vec3 &a1, const glm::vec3 &a2,
                                                                              const glm::vec3 &b1,
                                                                              const glm::vec3 &b2) {
    const glm::vec3 d1 = a2 - a1; // Direction vector of first segment
    const glm::vec3 d2 = b2 - b1; // Direction vector of second segment
    const glm::vec3 r  = a1 - b1; // Vector between starting points

    // Precompute dot products
    const float a = glm::dot(d1, d1); // Squared length of d1
    const float b = glm::dot(d1, d2); // Dot product of d1 and d2
    const float c = glm::dot(d2, d2); // Squared length of d2
    const float d = glm::dot(d1, r);  // Projection of r onto d1
    const float e = glm::dot(d2, r);  // Projection of r onto d2

    const float denom = a * c - b * b; // Denominator for linear system
    float s = 0.0f, t = 0.0f;

    if (denom < 1e-6f) { // Lines are parallel or degenerate
        // Handle degenerate cases first
        if (a < 1e-6f && c < 1e-6f) { // Both segments are points
            return { a1, b1 };
        }
        if (a < 1e-6f) { // First segment is a point
            t = glm::clamp(-e / c, 0.0f, 1.0f);
            return { a1, b1 + t * d2 };
        }
        if (c < 1e-6f) { // Second segment is a point
            s = glm::clamp(-d / a, 0.0f, 1.0f);
            return { a1 + s * d1, b1 };
        }

        // Parallel segments - find minimum endpoint combinations
        glm::vec3 best_p1, best_p2;
        float min_dist = FLT_MAX;

        // Project endpoints of A onto B
        const auto [t_a1, d_a1] = find_min(a1, b1, b2);
        const auto [t_a2, d_a2] = find_min(a2, b1, b2);
        const glm::vec3 p_a1    = b1 + t_a1 * d2;
        const glm::vec3 p_a2    = b1 + t_a2 * d2;

        // Project endpoints of B onto A
        const auto [s_b1, d_b1] = find_min(b1, a1, a2);
        const auto [s_b2, d_b2] = find_min(b2, a1, a2);
        const glm::vec3 p_b1    = a1 + s_b1 * d1;
        const glm::vec3 p_b2    = a1 + s_b2 * d1;

        // Compare all possible combinations
        const std::array<std::tuple<glm::vec3, glm::vec3, float>, 4> candidates = { std::make_tuple(a1, p_a1, d_a1),
                                                                                    std::make_tuple(a2, p_a2, d_a2),
                                                                                    std::make_tuple(p_b1, b1, d_b1),
                                                                                    std::make_tuple(p_b2, b2, d_b2) };

        for (const auto &[pt1, pt2, dist] : candidates) {
            if (dist < min_dist) {
                min_dist = dist;
                best_p1  = pt1;
                best_p2  = pt2;
            }
        }

        return { best_p1, best_p2 };
    } else { // General case (non-parallel lines)
        // Solve linear system using Cramer's rule
        s = glm::clamp((b * e - c * d) / denom, 0.0f, 1.0f);
        t = (b * s + e) / c;

        // Iterative clamping for numerical stability
        t = glm::clamp(t, 0.0f, 1.0f);
        s = glm::clamp((b * t - d) / a, 0.0f, 1.0f);

        return { a1 + s * d1, b1 + t * d2 };
    }
}

void CollisionSystem::get_capsule_endpoints(const Transform &t, const Collider &c, glm::vec3 &base, glm::vec3 &top) {
    // Calculate capsule center in world space
    const glm::vec3 center = t.position + c.offset;

    // Get orientation and calculate capsule direction
    const glm::vec3 world_up = t.orientation() * glm::vec3(0, 1, 0);

    // Calculate actual cylinder height (total_height - 2*radius)
    const float cylinder_height = glm::max(c.capsule_height - 2 * c.capsule_radius, 0.0f);
    const float half_cylinder   = cylinder_height * 0.5f;

    // Calculate endpoints relative to center
    base = center - world_up * half_cylinder;
    top  = center + world_up * half_cylinder;
}
