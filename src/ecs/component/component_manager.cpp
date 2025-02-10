#include <ecs/component/component_manager.h>

void ComponentManager::entity_destroyed(Entity entity) const {
    for (auto const &pair : m_component_arrays) {
        auto const &component = pair.second;
        component->entity_destroyed(entity);
    }
}