#include <glad.h>

#include <ecs/system/render.h>

void RenderSystem::draw_scene(entt::registry &registry, const std::shared_ptr<Scene> &scene,
                              const std::shared_ptr<FrameBuffer> &target) {
    // Set render target
    if (target) {
        target->bind();
        glViewport(0, 0, target->get_width(), target->get_height());
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Get view/projection matrices
    const auto &view   = scene->get_main_camera()->get_view_matrix();
    const auto &proj   = scene->get_main_camera()->get_projection_matrix();
    const auto &shader = scene->get_current_shader();
    shader->set_matrix4("uView", view);
    shader->set_matrix4("uProjection", proj);

    // Draw all renderable entities
    auto render_view = registry.view<const Transform, const Renderable>();
    render_view.each([&](const auto &transform, const auto &renderable) {
        shader->set_matrix4("uModel", transform.matrix());
        auto meshes = renderable.model->get_meshes();
        for (const auto &mesh : meshes) {
            glBindVertexArray(mesh->get_vao());
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh->get_indices_size()), GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
            for (const auto &texture : mesh->get_textures()) {
                auto offset = static_cast<int>(texture->get_type());
                shader->set_integer(
                    std::string("uMaterial").append(texture_type_to_string(texture->get_type())).c_str(), offset);
                glActiveTexture(GL_TEXTURE0 + offset);
                glBindTexture(GL_TEXTURE_2D, texture->get_id());
            }
        }
    });

    if (target) {
        target->unbind();
    }
}
