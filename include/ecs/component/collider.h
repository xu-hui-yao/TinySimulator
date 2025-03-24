#pragma once

#include <glm/glm.hpp>
#include <ecs/component/transform.h>

struct Collider {
    enum ShapeType { SPHERE, BOX, CAPSULE } shape;

    // Common properties
    glm::vec3 offset{ 0.0f };  // Local offset from entity's transform
    bool is_trigger   = false; // Trigger flag (no physical response)
    bool is_active    = true;  // Collision detection toggle
    float friction    = 0.5f;  // Surface friction coefficient (0-1)
    float restitution = 0.2f;  // Bounce coefficient (0=inelastic, 1=perfect bounce)
    union {
        struct {
            float radius;
        }; // Sphere parameters
        struct {
            glm::vec3 half_extents;
        }; // Box parameters (half sizes)
        struct {
            float capsule_radius, capsule_height;
        }; // Capsule parameters
    };

    /**
     * @brief Collision manifold containing collision resolution data
     */
    struct CollisionManifold {
        glm::vec3 normal;           // Collision normal pointing from A to B
        glm::vec3 contact_point;    // World-space contact position
        float penetration_depth;    // Overlap distance between objects
        float combined_friction;    // Mixed friction coefficient
        float combined_restitution; // Mixed bounce coefficient
    };

    /**
     * @brief Detect collision between two colliders
     * @param a_t Transform of first entity
     * @param a_c Collider of first entity
     * @param b_t Transform of second entity
     * @param b_c Collider of second entity
     * @param out [out] Collision manifold with resolution data
     * @return true if collision detected, false otherwise
     */
    static bool collide(const Transform &a_t, const Collider &a_c, const Transform &b_t, const Collider &b_c,
                        CollisionManifold &out);

private:
    // Collision detection primitives
    static bool sphere_vs_sphere(const Transform &a_t, const Collider &a_c, const Transform &b_t, const Collider &b_c,
                                 CollisionManifold &out);

    static bool sphere_vs_box(const Transform &sphere_t, const Collider &sphere_c, const Transform &box_t,
                              const Collider &box_c, CollisionManifold &out);

    static bool sphere_vs_capsule(const Transform &sphere_t, const Collider &sphere_c, const Transform &capsule_t,
                                  const Collider &capsule_c, CollisionManifold &out);

    static bool box_vs_box(const Transform &a_t, const Collider &a_c, const Transform &b_t, const Collider &b_c,
                           CollisionManifold &out);

    static bool box_vs_capsule(const Transform &box_t, const Collider &box_c, const Transform &capsule_t,
                               const Collider &capsule_c, CollisionManifold &out);

    static bool capsule_vs_capsule(const Transform &a_t, const Collider &a_c, const Transform &b_t, const Collider &b_c,
                                   CollisionManifold &out);

    // Geometric utilities
    static glm::vec3 closest_point_on_line_segment(const glm::vec3 &point, const glm::vec3 &a, const glm::vec3 &b);

    static std::pair<glm::vec3, glm::vec3> closest_points_between_lines(const glm::vec3 &a1, const glm::vec3 &a2,
                                                                        const glm::vec3 &b1, const glm::vec3 &b2);

    static void get_capsule_endpoints(const Transform &t, const Collider &c, glm::vec3 &base, glm::vec3 &top);
};
