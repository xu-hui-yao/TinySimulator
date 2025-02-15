// #pragma once
//
// #include <ecs/entity/entity_manager.h>
// #include <ecs/fwd.h>
// #include <imgui.h>
// #include <scene/scene.h>
//
// class HierarchyPanel {
// public:
//     HierarchyPanel() = default;
//
//     ~HierarchyPanel() = default;
//
//     void on_imgui_render();
//
//     // Set context for ECS operations
//     void set_context(const std::shared_ptr<Coordinator> &coordinator, const std::shared_ptr<Scene> &scene);
//
// private:
//     // Internal helper functions
//     void draw_entity_node(Entity entity);
//     void draw_entity_context_menu(Entity entity);
//     void draw_entity_drag_drop_target(Entity entity);
//     void handle_keyboard_shortcuts();
//
//     // ECS context
//     std::weak_ptr<Coordinator> m_coordinator;
//     std::weak_ptr<Scene> m_scene;
//
//     // Panel state
//     Entity m_selected_entity{INVALID_ENTITY};
//     ImGuiTextFilter m_filter;
//     bool m_show_panel{true};
// };