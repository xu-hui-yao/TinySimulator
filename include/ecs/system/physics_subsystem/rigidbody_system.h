#pragma once

#include <ecs/system/physics_subsystem/physics_subsystem.h>
#include <glm/glm.hpp>

class RigidBodySystem : public PhysicsSubsystem {
public:
    [[nodiscard]] int execution_priority() const override;

    void update(entt::registry &registry, float dt) override;

private:
    constexpr static int priority = 5;
    glm::vec3 gravity             = glm::vec3(0.0f, -9.81f, 0.0f);

    void integrate_forces(entt::registry &registry, float dt);
};
