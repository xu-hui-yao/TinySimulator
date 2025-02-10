#pragma once

#include <ecs/entity/entity_manager.h>
#include <set>

// Base system class tracking relevant entities
class System {
public:
    virtual ~System() = 0;

    virtual void init() = 0;

    virtual void update(float dt) = 0;

    std::set<Entity> m_entities; // Entities matching the system's signature
};