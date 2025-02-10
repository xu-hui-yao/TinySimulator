#pragma once

#include <core/global.h>
#include <ecs/component/component_array.h>
#include <ecs/entity/entity_manager.h>
#include <typeindex>

class ComponentManager {
public:
    template <typename T> void register_component() {
        auto type_index = std::type_index(typeid(T));

        if (m_component_types.contains(type_index)) {
            get_logger()->warn("Registering component type more than once.");
            return;
        }

        m_component_types.insert({ type_index, m_next_component_type });
        m_component_arrays.insert({ type_index, std::make_shared<ComponentArray<T>>() });
        ++m_next_component_type;
    }

    template <typename T> ComponentType get_component_type() {
        auto type_index = std::type_index(typeid(T));

        if (!m_component_types.contains(type_index)) {
            get_logger()->error("Component not registered before use.");
            return 0;
        }

        return m_component_types[type_index];
    }

    template <typename T> void add_component(Entity entity, T component) {
        get_component_array<T>()->insert_data(entity, component);
    }

    template <typename T> void remove_component(Entity entity) { get_component_array<T>()->remove_data(entity); }

    template <typename T> T &get_component(Entity entity) { return get_component_array<T>()->get_data(entity); }

    void entity_destroyed(Entity entity) const;

private:
    std::unordered_map<std::type_index, ComponentType> m_component_types{};
    std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> m_component_arrays{};
    ComponentType m_next_component_type{};

    template <typename T> std::shared_ptr<ComponentArray<T>> get_component_array() {
        auto type_index = std::type_index(typeid(T));

        if (!m_component_arrays.contains(type_index)) {
            get_logger()->error("Component not registered before use.");
            return nullptr;
        }

        return std::static_pointer_cast<ComponentArray<T>>(m_component_arrays[type_index]);
    }
};