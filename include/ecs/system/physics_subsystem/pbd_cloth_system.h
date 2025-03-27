#pragma once

#include <ecs/system/physics_subsystem/physics_subsystem.h>

class PBDClothSystem : public PhysicsSubsystem {
public:
    [[nodiscard]] int execution_priority() const override;

    void update(entt::registry &registry, float dt) override;

private:
    static void predict_positions(Cloth &cloth, float dt);

    static void handle_collisions(Cloth &cloth, float dt);

    static void solve_constraints(Cloth &cloth, float dt);

    static void update_positions(Cloth &cloth, float dt);

    constexpr static int priority          = 15;
    constexpr static int solver_iterations = 3;
};
