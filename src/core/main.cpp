#include <core/fwd.h>
#include <ecs/component/cloth.h>
#include <ecs/component/collider.h>
#include <ecs/component/rigidbody.h>
#include <ecs/system/input.h>
#include <ecs/system/physics.h>
#include <ecs/system/physics_subsystem/collision_system.h>
#include <ecs/system/physics_subsystem/pbd_cloth_system.h>
#include <ecs/system/physics_subsystem/rigidbody_system.h>
#include <ecs/system/render.h>
#include <scene/model/model_manager.h>
#include <scene/scene/scene.h>

void create_assets(const std::shared_ptr<Scene> &scene, entt::registry &registry) {
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
    model_collider.shape          = Collider::CAPSULE;
    model_collider.capsule_radius = 0.6f;
    model_collider.capsule_height = 3.4f;
    model_collider.offset         = glm::vec3(0.0f, 1.7f, 0.0f);
    model_collider.visualize      = true;
    model_collider.generate_visualize_model();
    registry.emplace<Collider>(model_entity, model_collider);
    scene->load_model("marry", "assets/Marry/Marry.obj");
    registry.emplace<Renderable>(model_entity, scene->get_model("marry"), Renderable::fill);
    scene->get_model("marry")->upload(nullptr);

    const auto ground_entity = registry.create();
    registry.emplace<Transform>(ground_entity, glm::vec3(0, 0, 0), glm::vec3(0), glm::vec3(1));
    scene->load_model("ground", std::string(ModelLoader::internal_prefix) + "plane1",
                      { { "width", 8.0f }, { "height", 8.0f }, { "segments_x", 1 }, { "segments_z", 1 } });
    registry.emplace<Renderable>(ground_entity, scene->get_model("ground"));
    scene->get_model("ground")->upload(nullptr);
    Collider ground_collider;
    ground_collider.shape          = Collider::BOX;
    ground_collider.half_extents   = glm::vec3(4.0f, 0.1f, 4.0f);
    ground_collider.visualize      = true;
    ground_collider.generate_visualize_model();
    registry.emplace<Collider>(ground_entity, ground_collider);

    int cloth_resolution    = 32;
    const auto cloth_entity = registry.create();
    Transform cloth_transform(glm::vec3(0, 10.0, 0), glm::vec3(0, 0, 0), glm::vec3(1));
    registry.emplace<Transform>(cloth_entity, cloth_transform);
    scene->load_model("cloth", std::string(ModelLoader::internal_prefix) + "plane2",
                      { { "width", 10.0f },
                        { "height", 10.0f },
                        { "segments_x", cloth_resolution },
                        { "segments_z", cloth_resolution } });
    auto cloth_model = scene->get_model("cloth");
    Cloth cloth_cloth(cloth_model, cloth_transform, 0.1f);
    cloth_cloth.fixed_vertices[0]                = true;
    cloth_cloth.fixed_vertices[cloth_resolution] = true;
    cloth_cloth.visualize                        = true;
    registry.emplace<Cloth>(cloth_entity, cloth_cloth);
    Renderable cloth_renderable(cloth_model, Renderable::polygon);
    registry.emplace<Renderable>(cloth_entity, cloth_renderable);
    cloth_model->upload(nullptr);
}

void init_physics() {
    PhysicsSystem::register_subsystem<RigidBodySystem>();
    PhysicsSystem::register_subsystem<CollisionSystem>();
    PhysicsSystem::register_subsystem<PBDClothSystem>();
}

int main() {
    get_window_manager()->init("test", 2560, 1440);
    init_physics();
    InputSystem::init();

    auto scene = get_root_scene();
    scene->load_shader("constant", "shaders/constant/constant.vert");
    scene->set_current_shader("constant");

    entt::registry registry;
    create_assets(scene, registry);

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
