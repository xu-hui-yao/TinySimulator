#include <ecs/system/system_manager.h>

template <typename T> std::shared_ptr<T> SystemManager::register_system() {
    auto type_index = std::type_index(typeid(T));

    if (m_systems.contains(type_index)) {
        get_logger()->warn("Registering system more than once.");
        return m_systems[type_index];
    }

    auto system = std::make_shared<T>();
    m_systems.insert({ type_index, system });
    return system;
}

template <typename T> void SystemManager::set_signature(Signature signature) {
    auto type_index = std::type_index(typeid(T));

    if (!m_systems.contains(type_index)) {
        get_logger()->error("System used before registered.");
        return;
    }

    m_signatures.insert({ type_index, signature });
}

void SystemManager::entity_destroyed(Entity entity) const {
    for (auto const &pair : m_systems) {
        auto const &system = pair.second;

        system->m_entities.erase(entity);
    }
}

void SystemManager::entity_signature_changed(Entity entity, Signature entity_signature) {
    for (auto const &pair : m_systems) {
        auto const &type             = pair.first;
        auto const &system           = pair.second;
        auto const &system_signature = m_signatures[type];

        if ((entity_signature & system_signature) == system_signature) {
            system->m_entities.insert(entity);
        } else {
            system->m_entities.erase(entity);
        }
    }
}