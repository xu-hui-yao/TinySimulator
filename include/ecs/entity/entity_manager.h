#pragma once

#include <array>
#include <bitset>
#include <core/global.h>
#include <queue>

using Entity    = std::uint32_t;
using Signature = std::bitset<M_MAX_COMPONENTS>;

/**
 * Manages entity lifecycle and component signatures.
 * Recycles entity IDs using a queue for efficient reuse.
 */
class EntityManager {
public:
    EntityManager();

    // Creates a new entity, recycling IDs if possible
    Entity create_entity();

    // Marks an entity for reuse and clears its signature
    void destroy_entity(Entity entity);

    // Updates the component signature for an entity
    void set_signature(Entity entity, Signature signature);

    // Retrieves the component signature of an entity
    [[nodiscard]] Signature get_signature(Entity entity) const;

private:
    std::queue<Entity> m_available_entities{};            // Recycled entity IDs
    std::array<Signature, M_MAX_ENTITIES> m_signatures{}; // Component signatures per entity
    std::uint32_t m_living_entity_count{ 0 };             // Active entity count
};
