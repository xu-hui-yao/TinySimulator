#pragma once

#include <entt/entt.hpp>

class PhysicsSystem {
public:
    static void update(entt::registry &registry, float delta_time);

private:
    struct CollisionInfo {
        glm::vec3 normal;
        float depth;
    };

    static bool check_collision(const Transform &a_t, const Collider &a_col, const Transform &b_t,
                                const Collider &b_col, CollisionInfo &out);

    static bool sphere_sphere(const glm::vec3 &a_pos, float a_rad, const glm::vec3 &b_pos, float b_rad,
                              CollisionInfo &out);

    static bool sphere_obb(const glm::vec3 &sphere_pos, float sphere_rad, const Transform &box_t,
                           const glm::vec3 &box_extents, CollisionInfo &out);

    static bool obb_obb(const Transform &a_t, const glm::vec3 &a_ext, const Transform &b_t, const glm::vec3 &b_ext,
                        CollisionInfo &out);

    static float project_obb(const glm::vec3 &axis, const glm::vec3 &center, const glm::vec3 *obb_axes,
                             const glm::vec3 &extents);
};