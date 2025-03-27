#pragma once

#include <entt/entt.hpp>

class PhysicsSubsystem {
public:
    virtual ~PhysicsSubsystem() = default;

    [[nodiscard]] virtual int execution_priority() const { return 0; }

    virtual void pre_update(entt::registry& registry, float dt) {}

    virtual void update(entt::registry& registry, float dt) = 0;

    virtual void post_update(entt::registry& registry, float dt) {}
};