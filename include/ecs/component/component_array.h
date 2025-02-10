#pragma once

#include <array>
#include <ecs/entity/entity_manager.h>
#include <unordered_map>

using ComponentType = std::uint8_t;

// Interface for type-erased component array access
class IComponentArray {
public:
    virtual ~IComponentArray()                   = default;
    virtual void entity_destroyed(Entity entity) = 0;
};

/**
 * Dense array storage for components of type T.
 * Maintains mappings between entities and array indices for O(1) access.
 */
template <typename T> class ComponentArray : public IComponentArray {
public:
    // Adds a component to an entity
    void insert_data(Entity entity, T component) {
        if (m_entity_to_index_map.contains(entity)) {
            get_logger()->error("Component added to same entity more than once.");
            return;
        }

        size_t new_index                 = m_size;
        m_entity_to_index_map[entity]    = new_index;
        m_index_to_entity_map[new_index] = entity;
        m_component_array[new_index]     = component;
        ++m_size;
    }

    // Removes a component from an entity
    void remove_data(Entity entity) {
        if (!m_entity_to_index_map.contains(entity)) {
            get_logger()->warn("Removing non-existent component.");
            return;
        }

        size_t index_of_removed_entity             = m_entity_to_index_map[entity];
        size_t index_of_last_element               = m_size - 1;
        m_component_array[index_of_removed_entity] = m_component_array[index_of_last_element];

        Entity entity_of_last_element                  = m_index_to_entity_map[index_of_last_element];
        m_entity_to_index_map[entity_of_last_element]  = index_of_removed_entity;
        m_index_to_entity_map[index_of_removed_entity] = entity_of_last_element;

        m_entity_to_index_map.erase(entity);
        m_index_to_entity_map.erase(index_of_last_element);

        --m_size;
    }

    // Retrieves a component for an entity
    T &get_data(Entity entity) {
        if (!m_entity_to_index_map.contains(entity)) {
            get_logger()->warn("Retrieving non-existent component.");
            return T();
        }

        return m_component_array[m_entity_to_index_map[entity]];
    }

    // Cleans up component data when an entity is destroyed
    void entity_destroyed(Entity entity) override {
        if (m_entity_to_index_map.contains(entity)) {
            remove_data(entity);
        }
    }

private:
    std::array<T, M_MAX_ENTITIES> m_component_array{};
    std::unordered_map<Entity, size_t> m_entity_to_index_map{}; // Entity -> dense array index
    std::unordered_map<size_t, Entity> m_index_to_entity_map{}; // Dense array index -> entity
    size_t m_size{ 0 };                                         // Current number of components
};
