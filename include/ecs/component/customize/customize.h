#pragma once

#include <ecs/entity/entity_manager.h>
#include <functional>

struct CustomizeComponent {
    std::function<void(Entity, float)> update_function;
};