#include <ecs/component/cloth.h>
#include <ecs/component/transform.h>
#include <ecs/system/physics_subsystem/pbd_cloth_system.h>

int PBDClothSystem::execution_priority() const { return priority; }

void PBDClothSystem::update(entt::registry &registry, float dt) {
    auto view = registry.view<Cloth, Transform>();
    view.each([&](Cloth &cloth, const Transform &transform) {
        // Phase 1: Predict positions with external forces
        predict_positions(cloth, dt);
        // Phase 2: Handle collisions
        handle_collisions(cloth, dt);
        // Phase 3: Solve constraints iteratively
        solve_constraints(cloth, dt);
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

void PBDClothSystem::handle_collisions(Cloth &cloth, float dt) {}

void PBDClothSystem::solve_constraints(Cloth &cloth, float dt) {
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
