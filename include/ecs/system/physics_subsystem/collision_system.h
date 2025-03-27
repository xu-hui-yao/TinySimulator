#pragma once

#include <ecs/component/collider.h>
#include <ecs/component/transform.h>
#include <ecs/system/physics_subsystem/physics_subsystem.h>

class CollisionSystem : public PhysicsSubsystem {
public:
    [[nodiscard]] int execution_priority() const override;

    void update(entt::registry &registry, float dt) override;

private:
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
    struct CollisionPair {
        entt::entity a;
        entt::entity b;
        CollisionManifold manifold;
    };
    std::vector<CollisionPair> collision_pairs;
    constexpr static int priority = 10;

    void detect_collisions(entt::registry &registry);

    void resolve_collisions(entt::registry &registry, float dt);

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

    static std::pair<float, float> find_min(const glm::vec3 &point, const glm::vec3 &seg_start,
                                            const glm::vec3 &seg_end);
};
