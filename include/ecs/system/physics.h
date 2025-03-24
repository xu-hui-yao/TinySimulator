#pragma once

#include <ecs/component/collider.h>
#include <ecs/component/transform.h>
#include <entt/entt.hpp>

class PhysicsSystem {
public:
    static void update(entt::registry &registry, float dt);

private:
    struct CollisionPair {
        entt::entity a;
        entt::entity b;
        Collider::CollisionManifold manifold;
    };
    static std::vector<CollisionPair> collision_pairs;

    // Physics simulation steps
    static void integrate_forces(entt::registry &registry, float dt);

    static void detect_collisions(entt::registry &registry);

    static void resolve_collisions(entt::registry &registry, float dt);

    // Physics parameters
    static constexpr glm::vec3 gravity{ 0.0f, -9.81f, 0.0f };
    static constexpr float epsilon = 0.01f; // Minimum separation distance
};