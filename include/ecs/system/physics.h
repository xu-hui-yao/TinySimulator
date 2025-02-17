#pragma once

#include <entt/entt.hpp>

class PhysicsSystem {
public:
    static void update(entt::registry& registry, float delta_time);
};