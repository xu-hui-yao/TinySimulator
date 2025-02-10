#include <ecs/coordinator.h>

void Coordinator::init() {
    m_component_manager = std::make_unique<ComponentManager>();
    m_entity_manager    = std::make_unique<EntityManager>();
    m_system_manager    = std::make_unique<SystemManager>();
}

[[nodiscard]] Entity Coordinator::create_entity() const { return m_entity_manager->create_entity(); }

void Coordinator::destroy_entity(Entity entity) const {
    m_entity_manager->destroy_entity(entity);
    m_component_manager->entity_destroyed(entity);
    m_system_manager->entity_destroyed(entity);
}