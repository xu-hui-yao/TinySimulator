#pragma once

#include <core/global.h>
#include <ecs/system/system.h>
#include <memory>
#include <typeindex>

/**
 * Manages system registration and entity membership based on signatures.
 */
class SystemManager {
public:
    // Registers a system type and returns its instance
    template <typename T> std::shared_ptr<T> register_system();

    // Sets the component signature required by a system
    template <typename T> void set_signature(Signature signature);

    // Removes an entity from all systems
    void entity_destroyed(Entity entity) const;

    // Updates system membership when an entity's signature changes
    void entity_signature_changed(Entity entity, Signature entity_signature);

private:
    std::unordered_map<std::type_index, Signature> m_signatures{};            // System signatures
    std::unordered_map<std::type_index, std::shared_ptr<System>> m_systems{}; // Registered systems
};