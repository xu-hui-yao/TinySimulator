#pragma once

#include <ecs/component/renderable.h>
#include <ecs/component/transform.h>
#include <entt/entt.hpp>
#include <renderer/frame_buffer.h>
#include <scene/camera/camera.h>

class RenderSystem {
public:
    static void draw_scene(entt::registry &registry, const std::shared_ptr<Camera> &camera,
                           const std::shared_ptr<FrameBuffer> &target = nullptr) {
        // Set render target
        if (target) {
            target->bind();
            glViewport(0, 0, target->get_width(), target->get_height());
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get view/projection matrices
        const auto view = camera->get_view_matrix();
        const auto proj = camera->get_projection_matrix();

        // Draw all renderable entities
        auto view = registry.view<const Transform, const Renderable>();
        view.each([&](const auto &transform, const auto &renderable) {
            if (renderable.model && renderable.model->IsLoaded()) {
                renderable.model->GetShader()->Use();
                renderable.model->GetShader()->SetMat4("u_View", view);
                renderable.model->GetShader()->SetMat4("u_Projection", proj);
                renderable.model->GetShader()->SetMat4("u_Model", transform.matrix());
                renderable.model->GetShader()->SetVec4("u_Color", renderable.color);
                renderable.model->Draw();
            }
        });

        if (target)
            target->unbind();
    }
};