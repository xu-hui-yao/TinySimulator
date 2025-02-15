#include <ecs/entity/entity_manager.h>

EntityManager::EntityManager() {
    for (Entity entity = 1; entity < M_MAX_ENTITIES; ++entity) {
        m_available_entities.push(entity);
    }
}

Entity EntityManager::create_entity() {
    if (m_living_entity_count >= M_MAX_ENTITIES) {
        get_logger()->error("Too many entities in existence.");
        return 0;
    }

    Entity id = m_available_entities.front();
    m_available_entities.pop();
    ++m_living_entity_count;

    return id;
}

void EntityManager::destroy_entity(Entity entity) {
    if (entity >= M_MAX_ENTITIES) {
        get_logger()->error("Entity out of range.");
        return;
    }

    m_signatures[entity].reset();
    m_available_entities.push(entity);
    --m_living_entity_count;
}

void EntityManager::set_signature(Entity entity, Signature signature) {
    if (entity >= M_MAX_ENTITIES) {
        get_logger()->error("Entity out of range.");
        return;
    }

    m_signatures[entity] = signature;
}

[[nodiscard]] Signature EntityManager::get_signature(Entity entity) const {
    if (entity >= M_MAX_ENTITIES) {
        get_logger()->error("Entity out of range.");
        return 0;
    }

    return m_signatures[entity];
}
