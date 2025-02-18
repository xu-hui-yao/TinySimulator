#pragma once

#include <entt/entt.hpp>

class InputSystem {
public:
    static void init();

    static void process(entt::registry &registry);
};
