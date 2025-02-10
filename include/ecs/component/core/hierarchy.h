#pragma once

#include <vector>
#include <ecs/entity/entity_manager.h>

struct HierarchyComponent {
    bool root;
    Entity parent;
    std::vector<Entity> children;
};