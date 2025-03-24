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
    registry.emplace<Transform>(model_entity, glm::vec3(0, 0, 0), glm::vec3(0), glm::vec3(1));
    RigidBody model_rb;
    model_rb.mass = 0.0f;
    registry.emplace<RigidBody>(model_entity, model_rb);
    Collider model_collider;
    model_collider.shape = Collider::CAPSULE;
    model_collider.capsule_radius = 0.6f;
    model_collider.capsule_height = 2.7f;
    registry.emplace<Collider>(model_entity, model_collider);
    scene->load_model("marry", "assets/Marry/Marry.obj");
    registry.emplace<Renderable>(model_entity, scene->get_model("marry"));
    scene->get_model("marry")->upload(nullptr);

    const auto ground_entity = registry.create();
    registry.emplace<Transform>(ground_entity, glm::vec3(0, 0, 0), glm::vec3(0), glm::vec3(1));
    scene->load_model("ground", std::string(ModelLoader::internal_prefix) + "plane1",
                      { { "width", 10.0f }, { "height", 10.0f }, { "segments_x", 1 }, { "segments_z", 1 } });
    registry.emplace<Renderable>(ground_entity, scene->get_model("ground"));
    scene->get_model("ground")->upload(nullptr);

    const auto cloth_entity = registry.create();
    registry.emplace<Transform>(cloth_entity, glm::vec3(0, 50.0, 0), glm::vec3(0), glm::vec3(1));
    scene->load_model("cloth", std::string(ModelLoader::internal_prefix) + "plane2",
                      { { "width", 10.0f }, { "height", 10.0f }, { "segments_x", 4 }, { "segments_z", 4 } });
    RigidBody cloth_rb;
    cloth_rb.mass = 1.0f;
    registry.emplace<RigidBody>(cloth_entity, cloth_rb);
    Collider cloth_collider;
    cloth_collider.shape = Collider::BOX;
    cloth_collider.half_extents = glm::vec3(5.0f, 0.1f, 5.0f);
    registry.emplace<Collider>(cloth_entity, cloth_collider);
    registry.emplace<Renderable>(cloth_entity, scene->get_model("cloth"));
    scene->get_model("cloth")->upload(nullptr);

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
