#include <bitset>
#include <core/event/event_manager.h>
#include <ecs/system/input.h>
#include <iostream>
#include <scene/scene/scene.h>

void InputSystem::init() {
    get_event_manager()->add_listener(Events::Window::INPUT, [](Event &event) {
        auto keys   = event.get_param<std::bitset<10>>(Events::Window::Input::KEYBOARD_INPUT);
        auto camera = get_root_scene()->get_main_camera();

        if (keys[static_cast<int>(InputButtons::W)])
            camera->move_forward(get_delta_time());
        if (keys[static_cast<int>(InputButtons::S)])
            camera->move_backward(get_delta_time());
        if (keys[static_cast<int>(InputButtons::A)])
            camera->move_left(get_delta_time());
        if (keys[static_cast<int>(InputButtons::D)])
            camera->move_right(get_delta_time());
        if (keys[static_cast<int>(InputButtons::SPACE)])
            camera->move_up(get_delta_time());
        if (keys[static_cast<int>(InputButtons::SHIFT)])
            camera->move_down(get_delta_time());

        auto mouse_pos_offset = event.get_param<glm::vec2>(Events::Window::Input::MOUSE_POSITION);
        camera->update_orientation(mouse_pos_offset);
    });
}

void InputSystem::process(entt::registry &registry) {}
