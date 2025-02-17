#include <core/fwd.h>
#include <ecs/system/render.h>
#include <ecs/system/physics.h>

int main() {
    get_window_manager()->init("test", 1920, 1080);

    // 创建ECS Registry
    entt::registry registry;

    // 创建基础实体
    const auto cameraEntity = registry.create();
    auto& camera = registry.emplace<Camera>(
        cameraEntity,
        glm::vec3(0, 5, 10),
        glm::vec3(0),
        glm::vec3(0, 1, 0),
        60.0f,
        16.0f/9.0f,
        0.1f,
        100.0f
    );

    // 创建测试立方体
    const auto cubeEntity = registry.create();
    registry.emplace<Transform>(cubeEntity,
        glm::vec3(0, 5, 0), glm::vec3(0), glm::vec3(1));
    registry.emplace<RigidBody>(cubeEntity,
        glm::vec3(0), 1.0f, true);
    registry.emplace<Renderable>(cubeEntity,
        get_model_manager()->Load("cube", "assets/cube.obj"),
        glm::vec4(1,0,0,1));

    while (!get_quit()) {
        auto start_time = std::chrono::high_resolution_clock::now();

        get_window_manager()->process_events();

        // 更新物理
        PhysicsSystem::update(registry, get_delta_time());

        // 渲染场景
        RenderSystem::draw_scene(registry, get_main_camera());

        get_window_manager()->update();

        auto stop_time = std::chrono::high_resolution_clock::now();
        get_delta_time() = std::chrono::duration<float>(stop_time - start_time).count();
    }

    get_window_manager()->shutdown();
    return 0;
}
