#pragma once

#include <algorithm>
#include <ecs/system/physics_subsystem/physics_subsystem.h>
#include <memory>
#include <vector>

class PhysicsSystem {
public:
    template <typename T, typename... Args> static void register_subsystem(Args &&...args) {
        subsystems.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
        std::sort(subsystems.begin(), subsystems.end(),
                  [](const auto &a, const auto &b) { return a->execution_priority() < b->execution_priority(); });
    }

    static void update(entt::registry &registry, float dt);

private:
    static std::vector<std::unique_ptr<PhysicsSubsystem>> subsystems;
};
