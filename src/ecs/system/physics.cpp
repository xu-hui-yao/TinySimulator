#include <ecs/system/physics.h>

std::vector<std::unique_ptr<PhysicsSubsystem>> PhysicsSystem::subsystems;

void PhysicsSystem::update(entt::registry &registry, float dt) {
    for (auto &sys : subsystems)
        sys->pre_update(registry, dt);
    for (auto &sys : subsystems)
        sys->update(registry, dt);
    for (auto &sys : subsystems)
        sys->post_update(registry, dt);
}
