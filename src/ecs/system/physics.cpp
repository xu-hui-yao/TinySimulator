#pragma once

#include <ecs/component/collider.h>
#include <ecs/component/rigidbody.h>
#include <ecs/component/transform.h>
#include <ecs/system/physics.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

void PhysicsSystem::update(entt::registry &registry, float delta_time) {
    constexpr glm::vec3 gravity{ 0.0f, -9.81f, 0.0f };

    // 第一阶段：应用物理运动
    auto moving_view = registry.view<Transform, RigidBody, Collider>();
    moving_view.each([&](auto entity, auto &t, auto &rb, auto &col) {
        if (rb.use_gravity) {
            rb.velocity += gravity * delta_time;
        }

        const glm::vec3 prev_pos = t.position;
        t.position += rb.velocity * delta_time;

        // 碰撞检测与响应
        registry.view<Transform, Collider>().each([&](auto other_entity, auto &other_t, auto &other_col) {
            if (entity == other_entity || !col.is_active || !other_col.is_active)
                return;

            CollisionInfo collision;
            if (check_collision(t, col, other_t, other_col, collision)) {
                if (col.is_trigger || other_col.is_trigger) {
                    // 触发事件处理
                    return;
                }

                // 简单碰撞响应
                t.position = prev_pos;
                rb.velocity -= collision.normal * glm::dot(rb.velocity, collision.normal) * 1.8f;
            }
        });
    });
}

bool PhysicsSystem::check_collision(const Transform &a_t, const Collider &a_col, const Transform &b_t,
                                    const Collider &b_col, CollisionInfo &out) {
    const glm::vec3 a_pos = a_t.position + a_col.offset;
    const glm::vec3 b_pos = b_t.position + b_col.offset;

    // 形状组合分发
    if (a_col.shape_type == Collider::SPHERE) {
        if (b_col.shape_type == Collider::SPHERE) {
            return sphere_sphere(a_pos, a_col.radius, b_pos, b_col.radius, out);
        }
        if (b_col.shape_type == Collider::BOX) {
            return sphere_obb(a_pos, a_col.radius, b_t, b_col.half_extents, out);
        }
    }

    if (a_col.shape_type == Collider::BOX && b_col.shape_type == Collider::BOX) {
        return obb_obb(a_t, a_col.half_extents, b_t, b_col.half_extents, out);
    }

    // 其他组合处理...
    return false;
}

bool PhysicsSystem::sphere_sphere(const glm::vec3 &a_pos, float a_rad, const glm::vec3 &b_pos, float b_rad,
                                  CollisionInfo &out) {
    const glm::vec3 delta = b_pos - a_pos;
    const float dist_sq   = glm::dot(delta, delta);
    const float min_dist  = a_rad + b_rad;

    if (dist_sq < min_dist * min_dist) {
        const float dist = std::sqrt(dist_sq);
        out.normal       = dist > 0.001f ? delta / dist : glm::vec3(0, 1, 0);
        out.depth        = min_dist - dist;
        return true;
    }
    return false;
}

bool PhysicsSystem::sphere_obb(const glm::vec3 &sphere_pos, float sphere_rad, const Transform &box_t,
                               const glm::vec3 &box_extents, CollisionInfo &out) {
    const glm::mat3 rot        = box_t.orientation_matrix();
    const glm::vec3 box_center = box_t.position;

    // 转换到OBB局部空间
    const glm::vec3 local_sphere = glm::transpose(rot) * (sphere_pos - box_center);

    // 最近点计算
    const glm::vec3 closest = glm::clamp(local_sphere, -box_extents, box_extents);
    const glm::vec3 delta   = local_sphere - closest;
    const float dist_sq     = glm::dot(delta, delta);

    if (dist_sq < sphere_rad * sphere_rad) {
        const float dist = std::sqrt(dist_sq);
        out.depth        = sphere_rad - dist;
        out.normal       = rot * (dist > 0.001f ? delta / dist : glm::vec3(0, 1, 0));
        out.normal       = glm::normalize(out.normal);
        return true;
    }
    return false;
}

bool PhysicsSystem::obb_obb(const Transform &a_t, const glm::vec3 &a_ext, const Transform &b_t, const glm::vec3 &b_ext,
                            CollisionInfo &out) {
    const glm::mat3 a_rot = a_t.orientation_matrix();
    const glm::mat3 b_rot = b_t.orientation_matrix();

    const glm::vec3 a_axes[3] = { a_rot[0], a_rot[1], a_rot[2] };
    const glm::vec3 b_axes[3] = { b_rot[0], b_rot[1], b_rot[2] };

    const glm::vec3 center_a = a_t.position;
    const glm::vec3 center_b = b_t.position;
    const glm::vec3 delta    = center_b - center_a;

    float min_overlap = FLT_MAX;
    glm::vec3 best_axis;

    // 测试所有分离轴
    auto test_axis = [&](const glm::vec3 &axis, float epsilon = 0.001f) {
        if (glm::length(axis) < epsilon)
            return true;

        const glm::vec3 norm_axis = glm::normalize(axis);

        const float a_proj  = project_obb(norm_axis, center_a, a_axes, a_ext);
        const float b_proj  = project_obb(norm_axis, center_b, b_axes, b_ext);
        const float overlap = a_proj + b_proj - glm::abs(glm::dot(delta, norm_axis));

        if (overlap < 0)
            return false;
        if (overlap < min_overlap) {
            min_overlap = overlap;
            best_axis   = norm_axis;
        }
        return true;
    };

    // 测试所有可能的分离轴
    for (const auto &axis : a_axes) {
        if (!test_axis(axis))
            return false;
    }
    for (const auto &axis : b_axes) {
        if (!test_axis(axis))
            return false;
    }
    for (const auto &a : a_axes) {
        for (const auto &b : b_axes) {
            const glm::vec3 cross_axis = glm::cross(a, b);
            if (!test_axis(cross_axis))
                return false;
        }
    }

    // 确定法线方向
    out.normal = glm::dot(delta, best_axis) < 0 ? best_axis : -best_axis;
    out.depth  = min_overlap;
    return true;
}

float PhysicsSystem::project_obb(const glm::vec3 &axis, const glm::vec3 &center, const glm::vec3 *obb_axes,
                             const glm::vec3 &extents) {
    return extents.x * glm::abs(glm::dot(axis, obb_axes[0])) + extents.y * glm::abs(glm::dot(axis, obb_axes[1])) +
           extents.z * glm::abs(glm::dot(axis, obb_axes[2]));
}