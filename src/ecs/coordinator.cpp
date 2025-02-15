#include <ecs/coordinator.h>

Coordinator::Coordinator() {
    m_component_manager = std::make_unique<ComponentManager>();
    m_entity_manager    = std::make_unique<EntityManager>();
    m_system_manager    = std::make_unique<SystemManager>();
    m_event_manager     = std::make_unique<EventManager>();
}

[[nodiscard]] Entity Coordinator::create_entity() const { return m_entity_manager->create_entity(); }

void Coordinator::destroy_entity(Entity entity) const {
    m_entity_manager->destroy_entity(entity);
    m_component_manager->entity_destroyed(entity);
    m_system_manager->entity_destroyed(entity);
}

void Coordinator::update_system(float dt) const {
    m_system_manager->update_all(dt);
}

void Coordinator::add_event_listener(EventId event_id, std::function<void(Event &)> const &listener) const {
    m_event_manager->add_listener(event_id, listener);
}

void Coordinator::send_event(Event &event) const { m_event_manager->send_event(event); }

void Coordinator::send_event(EventId eventId) const { m_event_manager->send_event(eventId); }