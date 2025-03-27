#pragma once

#include <ecs/component/renderable.h>
#include <entt/entt.hpp>
#include <renderer/frame_buffer.h>
#include <scene/scene/scene.h>

class RenderSystem {
public:
    static void draw_scene(entt::registry &registry, const std::shared_ptr<Scene> &scene,
                           const std::shared_ptr<FrameBuffer> &target = nullptr);

private:
    static void draw_mesh(const std::vector<std::shared_ptr<Mesh>> &meshes, const std::shared_ptr<Shader> &shader);
};