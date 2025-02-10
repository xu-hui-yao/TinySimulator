#include <ecs/component/core/hierarchy.h>
#include <ecs/component/core/tag.h>
#include <ecs/component/core/transform.h>
#include <ecs/component/customize/customize.h>
#include <ecs/component/debug/debug.h>
#include <ecs/component/physics/collider.h>
#include <ecs/component/physics/rigid_body.h>
#include <ecs/component/render/camera.h>
#include <ecs/component/render/light.h>
#include <ecs/component/render/model.h>
#include <ecs/component/render/texture.h>
#include <ecs/component/ui/text.h>

#include <ecs/system/render/render_system.h>

#include <ecs/fwd.h>

std::shared_ptr<Coordinator> get_coordinator() {
    static auto coordinator = []() {
        auto coordinator = std::make_shared<Coordinator>();
        coordinator->register_component<TransformComponent>();
        coordinator->register_component<TagComponent>();
        coordinator->register_component<HierarchyComponent>();
        coordinator->register_component<ModelComponent>();
        coordinator->register_component<TextureComponent>();
        coordinator->register_component<CameraComponent>();
        coordinator->register_component<LightComponent>();
        coordinator->register_component<RigidBodyComponent>();
        coordinator->register_component<ColliderComponent>();
        coordinator->register_component<DebugComponent>();
        coordinator->register_component<UITextComponent>();
        coordinator->register_component<CustomizeComponent>();
        return coordinator;
    }();
    return coordinator;
}