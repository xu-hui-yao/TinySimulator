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

    // Get view/projection matrices
    const auto &view   = scene->get_main_camera()->get_view_matrix();
    const auto &proj   = scene->get_main_camera()->get_projection_matrix();
    const auto &shader = scene->get_current_shader();
    shader->set_matrix4("view", view);
    shader->set_matrix4("projection", proj);

    // Draw all renderable entities
    auto render_view = registry.view<const Transform, const Renderable>();
    render_view.each([&](const auto &transform, const auto &renderable) {
        shader->set_matrix4("model", transform.matrix());
        auto meshes = renderable.model->get_meshes();
        for (const auto &mesh : meshes) {
            glBindVertexArray(mesh->get_vao());
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh->get_indices_size()), GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
    });

    if (target) {
        target->unbind();
    }
}
