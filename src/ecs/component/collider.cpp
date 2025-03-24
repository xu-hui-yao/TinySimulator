#include <ecs/component/collider.h>
#include <glm/gtx/quaternion.hpp>

bool Collider::collide(const Transform &a_t, const Collider &a_c, const Transform &b_t, const Collider &b_c,
                       CollisionManifold &out) {
    const auto shape1 = a_c.shape;
    const auto shape2 = b_c.shape;
    // Handle shape order permutations
    if (shape1 == SPHERE) {
        if (shape2 == SPHERE)
            return sphere_vs_sphere(a_t, a_c, b_t, b_c, out);
        if (shape2 == BOX)
            return sphere_vs_box(a_t, a_c, b_t, b_c, out);
        if (shape2 == CAPSULE)
            return sphere_vs_capsule(a_t, a_c, b_t, b_c, out);
    } else if (shape1 == BOX) {
        if (shape2 == SPHERE)
            return sphere_vs_box(b_t, b_c, a_t, a_c, out);
        if (shape2 == BOX)
            return box_vs_box(a_t, a_c, b_t, b_c, out);
        if (shape2 == CAPSULE)
            return box_vs_capsule(a_t, a_c, b_t, b_c, out);
    } else if (shape1 == CAPSULE) {
        if (shape2 == SPHERE)
            return sphere_vs_capsule(b_t, b_c, a_t, a_c, out);
        if (shape2 == BOX)
            return box_vs_capsule(b_t, b_c, a_t, a_c, out);
        if (shape2 == CAPSULE)
            return capsule_vs_capsule(a_t, a_c, b_t, b_c, out);
    }
    return false;
}

void Collider::get_capsule_endpoints(const Transform &t, const Collider &c, glm::vec3 &base, glm::vec3 &top) {
    base = t.position + c.offset;
    // Get orientation and calculate capsule direction
    glm::vec3 local_up = glm::vec3(0, 1, 0);
    glm::vec3 world_up = t.orientation() * local_up;
    top                = base + world_up * c.capsule_height;
}

bool Collider::sphere_vs_capsule(const Transform &sphere_t, const Collider &sphere_c, const Transform &capsule_t,
                                 const Collider &capsule_c, CollisionManifold &out) {
    // Calculate capsule endpoints with rotation
    glm::vec3 capsule_base, capsule_top;
    get_capsule_endpoints(capsule_t, capsule_c, capsule_base, capsule_top);
    // Sphere center in world space
    const glm::vec3 sphere_center = sphere_t.position + sphere_c.offset;
    // Find closest point on capsule segment
    const glm::vec3 closest     = closest_point_on_line_segment(sphere_center, capsule_base, capsule_top);
    const glm::vec3 delta       = sphere_center - closest;
    const float dist            = glm::length(delta);
    const float combined_radius = sphere_c.radius + capsule_c.capsule_radius;
    if (dist < combined_radius) {
        out.normal               = glm::normalize(delta);
        out.contact_point        = closest + out.normal * capsule_c.capsule_radius;
        out.penetration_depth    = combined_radius - dist;
        out.combined_friction    = std::sqrt(sphere_c.friction * capsule_c.friction);
        out.combined_restitution = std::sqrt(sphere_c.restitution * capsule_c.restitution);
        return true;
    }
    return false;
}

bool Collider::box_vs_capsule(const Transform &box_t, const Collider &box_c, const Transform &capsule_t,
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

bool Collider::capsule_vs_capsule(const Transform &a_t, const Collider &a_c, const Transform &b_t, const Collider &b_c,
                                  CollisionManifold &out) {
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

bool Collider::sphere_vs_sphere(const Transform &a_t, const Collider &a_c, const Transform &b_t, const Collider &b_c,
                                CollisionManifold &out) {
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

bool Collider::box_vs_box(const Transform &a_t, const Collider &a_c, const Transform &b_t, const Collider &b_c,
                          CollisionManifold &out) {
    // 转换到A的局部空间
    glm::mat4 a_inv  = glm::inverse(a_t.matrix());
    glm::mat4 b_to_a = a_inv * b_t.matrix();

    // 获取B的轴对齐包围盒在A空间中的表示
    auto b_center       = glm::vec3(b_to_a[3]);
    glm::vec3 b_extents = b_c.half_extents;

    // 分离轴测试
    glm::vec3 a_extents = a_c.half_extents;
    glm::vec3 delta     = b_center;

    float penetration = FLT_MAX;
    glm::vec3 best_normal;

    // 测试A的坐标轴
    for (int i = 0; i < 3; ++i) {
        float overlap = a_extents[i] + b_extents[i] - std::abs(delta[i]);
        if (overlap < 0)
            return false;
        if (overlap < penetration) {
            penetration    = overlap;
            best_normal    = glm::vec3(0);
            best_normal[i] = delta[i] > 0 ? 1 : -1;
        }
    }

    // 测试B的坐标轴（在A空间中）
    glm::mat3 b_rot(b_to_a);
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

    // 找到最佳法线后处理接触点
    if (penetration < FLT_MAX) {
        // 转换法线回世界空间
        best_normal = glm::normalize(glm::vec3(a_t.matrix() * glm::vec4(best_normal, 0.0f)));

        out.normal               = best_normal;
        out.penetration_depth    = penetration;
        out.contact_point        = a_t.position + a_c.offset - best_normal * penetration * 0.5f;
        out.combined_friction    = std::sqrt(a_c.friction * b_c.friction);
        out.combined_restitution = std::sqrt(a_c.restitution * b_c.restitution);
        return true;
    }

    return false;
}

bool Collider::sphere_vs_box(const Transform &sphere_t, const Collider &sphere_c, const Transform &box_t,
                             const Collider &box_c, CollisionManifold &out) {
    // Convert to box's local space
    const glm::mat4 inv_box       = glm::inverse(box_t.matrix());
    const glm::vec3 sphere_center = glm::vec3(inv_box * glm::vec4(sphere_t.position + sphere_c.offset, 1.0f));
    // Find the closest point in box space
    const glm::vec3 closest_local = glm::clamp(sphere_center, -box_c.half_extents, box_c.half_extents);
    // Convert back to world space
    const glm::vec3 closest_world = box_t.matrix() * glm::vec4(closest_local, 1.0f);
    const glm::vec3 sphere_world  = sphere_t.position + sphere_c.offset;

    const glm::vec3 delta = sphere_world - closest_world;
    const float distance  = glm::length(delta);
    const float radius    = sphere_c.radius;
    if (distance < radius) {
        out.normal               = glm::normalize(delta);
        out.contact_point        = closest_world;
        out.penetration_depth    = radius - distance;
        out.combined_friction    = std::sqrt(sphere_c.friction * box_c.friction);
        out.combined_restitution = std::sqrt(sphere_c.restitution * box_c.restitution);
        return true;
    }
    return false;
}

glm::vec3 Collider::closest_point_on_line_segment(const glm::vec3 &point, const glm::vec3 &a, const glm::vec3 &b) {
    glm::vec3 ab = b - a;
    float t      = glm::dot(point - a, ab) / glm::dot(ab, ab);
    t            = glm::clamp(t, 0.0f, 1.0f);
    return a + t * ab;
}

std::pair<glm::vec3, glm::vec3> Collider::closest_points_between_lines(const glm::vec3 &a1, const glm::vec3 &a2,
                                                                       const glm::vec3 &b1, const glm::vec3 &b2) {
    // 使用几何方法计算最近点对
    glm::vec3 d1 = a2 - a1;
    glm::vec3 d2 = b2 - b1;
    glm::vec3 r  = b1 - a1;

    float a     = glm::dot(d1, d1);
    float b     = glm::dot(d1, d2);
    float c     = glm::dot(d2, d2);
    float d     = glm::dot(d1, r);
    float e     = glm::dot(d2, r);
    float denom = a * c - b * b;

    float s = 0.0f, t = 0.0f;
    if (denom != 0.0f) {
        s = glm::clamp((b * e - c * d) / denom, 0.0f, 1.0f);
    }
    t = (b * s + e) / c;
    t = glm::clamp(t, 0.0f, 1.0f);

    return { a1 + s * d1, b1 + t * d2 };
}
