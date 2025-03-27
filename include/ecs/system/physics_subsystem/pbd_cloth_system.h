#pragma once

#include <ecs/component/collider.h>
#include <ecs/system/physics_subsystem/physics_subsystem.h>

class PBDClothSystem : public PhysicsSubsystem {
public:
    [[nodiscard]] int execution_priority() const override;

    void update(entt::registry &registry, float dt) override;

private:
    static void predict_positions(Cloth &cloth, float dt);

    static void handle_collisions(Cloth &cloth, const Collider &collider, const Transform &collider_transform);

    static void solve_constraints(Cloth &cloth);

    static void update_positions(Cloth &cloth, float dt);

    static bool collide_sphere(const glm::vec3 &point, const Collider &collider, const Transform &transform,
                               glm::vec3 &surface_pos, glm::vec3 &normal);

    static bool collide_box(const glm::vec3 &point, const Collider &collider, const Transform &transform,
                            glm::vec3 &surface_pos, glm::vec3 &normal);

    static bool collide_capsule(const glm::vec3 &point, const Collider &collider, const Transform &transform,
                                glm::vec3 &surface_pos, glm::vec3 &normal);

    constexpr static int priority          = 15;
    constexpr static int solver_iterations = 3;
};
