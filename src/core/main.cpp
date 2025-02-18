#include <core/fwd.h>
#include <ecs/component/rigidbody.h>
#include <ecs/system/input.h>
#include <ecs/system/physics.h>
#include <ecs/system/render.h>
#include <scene/model/model_manager.h>
#include <scene/scene/scene.h>

int main() {
    get_window_manager()->init("test", 1920, 1080);

    InputSystem::init();

    auto scene = get_root_scene();

    entt::registry registry;

    scene->load_shader("blinn_phong", "shaders/blinn_phong/blinn_phong.vert");
    scene->set_current_shader("blinn_phong");

    const auto camera_entity = registry.create();
    auto &camera = registry.emplace<Camera>(camera_entity, glm::vec3(0, 5, 10), glm::vec3(0), glm::vec3(0, 1, 0), 60.0f,
                                            16.0f / 9.0f, 0.1f, 100.0f);
    scene->set_camera("main", std::make_shared<Camera>(camera));
    scene->set_main_camera("main");

    const auto model_entity = registry.create();
    registry.emplace<Transform>(model_entity, glm::vec3(0, 5, 0), glm::vec3(0), glm::vec3(1));
    registry.emplace<RigidBody>(model_entity, glm::vec3(0), 1.0f, true);
    scene->load_model("marry", "assets/Marry/Marry.obj");
    registry.emplace<Renderable>(model_entity, scene->get_model("marry"));
    scene->get_model("marry")->upload(nullptr);

    while (!get_quit()) {
        auto start_time = std::chrono::high_resolution_clock::now();

        get_window_manager()->process_events();

        PhysicsSystem::update(registry, get_delta_time());
        RenderSystem::draw_scene(registry, scene);

        get_window_manager()->update();

        auto stop_time   = std::chrono::high_resolution_clock::now();
        get_delta_time() = std::chrono::duration<float>(stop_time - start_time).count();
    }

    get_window_manager()->shutdown();
    return 0;
}
