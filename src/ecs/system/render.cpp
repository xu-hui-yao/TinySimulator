#include <glad.h>

#include <ecs/component/cloth.h>
#include <ecs/component/collider.h>
#include <ecs/component/transform.h>
#include <ecs/system/render.h>
#include <glm/ext/matrix_transform.hpp>

void RenderSystem::draw_scene(entt::registry &registry, const std::shared_ptr<Scene> &scene,
                              const std::shared_ptr<FrameBuffer> &target) {
    // Set render target
    if (target) {
        target->bind();
        glViewport(0, 0, target->get_width(), target->get_height());
    }

    // Set line width for better visibility
    glLineWidth(2.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Get view/projection matrices
    const auto &view   = scene->get_main_camera()->get_view_matrix();
    const auto &proj   = scene->get_main_camera()->get_projection_matrix();
    const auto &shader = scene->get_current_shader();
    shader->set_matrix4("uView", view);
    shader->set_matrix4("uProjection", proj);

    // Draw colliders in wireframe mode
    auto collider_view = registry.view<const Transform, const Collider>();
    collider_view.each([&](const auto &transform, const auto &collider) {
        if (collider.visualize) {
            // Enable polygon mode for wireframe rendering
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            shader->set_matrix4("uModel", glm::translate(transform.matrix(), collider.offset));
            draw_mesh(collider.visualize_model->get_meshes(), shader);

            // Reset polygon mode to fill after drawing colliders
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    });

    // Draw cloth mesh in polygon mode
    auto cloth_view = registry.view<const Transform, const Cloth>();
    cloth_view.each([&](const auto &transform, const auto &cloth) {
        if (cloth.visualize) {
            // Enable polygon mode for wireframe rendering
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            shader->set_matrix4("uModel", transform.matrix());
            draw_mesh(cloth.model->get_meshes(), shader);

            // Reset polygon mode to fill after drawing colliders
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    });

    // Draw all renderable entities
    auto render_view = registry.view<const Transform, const Renderable>();
    render_view.each([&](const auto &transform, const auto &renderable) {
        if (renderable.mode == Renderable::polygon) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        shader->set_matrix4("uModel", transform.matrix());
        draw_mesh(renderable.model->get_meshes(), shader);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    });

    if (target) {
        target->unbind();
    }
}

void RenderSystem::draw_mesh(const std::vector<std::shared_ptr<Mesh>> &meshes, const std::shared_ptr<Shader> &shader) {
    for (const auto &mesh : meshes) {
        glBindVertexArray(mesh->get_vao());
        for (const auto &texture : mesh->get_textures()) {
            auto offset = static_cast<int>(texture->get_type());
            shader->set_integer(std::string("uMaterial").append(texture_type_to_string(texture->get_type())).c_str(),
                                offset);
            glActiveTexture(GL_TEXTURE0 + offset);
            glBindTexture(GL_TEXTURE_2D, texture->get_id());
        }
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh->get_indices_size()), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
}
