#pragma once

#include <ecs/component/component_manager.h>
#include <ecs/entity/entity_manager.h>
#include <ecs/system/system_manager.h>
#include <memory>

class Coordinator {
public:
    void init();

    // Entity part

    [[nodiscard]] Entity create_entity() const;

    void destroy_entity(Entity entity) const;

    // Component part

    template <typename T> void register_component() const { m_component_manager->register_component<T>(); }

    template <typename T> void add_component(Entity entity, T component) {
        m_component_manager->add_component<T>(entity, component);

        auto signature = m_entity_manager->get_signature(entity);
        signature.set(m_component_manager->get_component_type<T>(), true);
        m_entity_manager->set_signature(entity, signature);

        m_system_manager->entity_signature_changed(entity, signature);
    }

    template <typename T> void remove_component(Entity entity) const {
        m_component_manager->remove_component<T>(entity);

        auto signature = m_entity_manager->get_signature(entity);
        signature.set(m_component_manager->get_component_type<T>(), false);
        m_entity_manager->set_signature(entity, signature);

        m_system_manager->entity_signature_changed(entity, signature);
    }

    template <typename T> T &get_component(Entity entity) { return m_component_manager->get_component<T>(entity); }

    template <typename T> [[nodiscard]] ComponentType get_component_type() const {
        return m_component_manager->get_component_type<T>();
    }

    // System part

    template <typename T> std::shared_ptr<T> register_system() { return m_system_manager->register_system<T>(); }

    template <typename T> void set_system_signature(Signature signature) const {
        m_system_manager->set_signature<T>(signature);
    }

private:
    std::unique_ptr<ComponentManager> m_component_manager;
    std::unique_ptr<EntityManager> m_entity_manager;
    std::unique_ptr<SystemManager> m_system_manager;
};