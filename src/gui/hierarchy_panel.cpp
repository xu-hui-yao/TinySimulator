// #include <ecs/coordinator.h>
// #include <gui/hierarchy_panel.h>
// #include <scene/scene.h>
//
// void HierarchyPanel::set_context(const std::shared_ptr<Coordinator> &coordinator, const std::shared_ptr<Scene> &scene) {
//     m_coordinator = coordinator;
//     m_scene       = scene;
// }
//
// void HierarchyPanel::on_imgui_render() {
//     if (!m_show_panel)
//         return;
//
//     ImGui::Begin("Hierarchy##panel", &m_show_panel, ImGuiWindowFlags_MenuBar);
//
//     // Menu Bar
//     if (ImGui::BeginMenuBar()) {
//         if (ImGui::BeginMenu("Add")) {
//             if (ImGui::MenuItem("Empty Entity")) {
//                 if (auto scene = m_scene.lock()) {
//                     scene->create_entity("New Entity");
//                 }
//             }
//             // Add more entity creation options
//             ImGui::EndMenu();
//         }
//         ImGui::EndMenuBar();
//     }
//
//     // Search Filter
//     m_filter.Draw("##Filter", ImGui::GetContentRegionAvail().x);
//
//     // Entity List
//     if (auto scene = m_scene.lock()) {
//         const auto &registry = scene->GetRegistry();
//
//         registry.each([&](auto entityID) {
//             Entity entity{ entityID, scene };
//
//             // Only show root entities (entities without parent)
//             if (!entity.HasComponent<Hierarchy>() || !entity.GetComponent<Hierarchy>().parent.IsValid()) {
//                 draw_entity_node(entity);
//             }
//         });
//     }
//
//     // Handle right-click on blank space
//     if (ImGui::BeginPopupContextWindow()) {
//         if (ImGui::MenuItem("Create Empty Entity")) {
//             if (auto scene = m_scene.lock()) {
//                 scene->CreateEntity("New Entity");
//             }
//         }
//         ImGui::EndPopup();
//     }
//
//     ImGui::End();
// }
//
// void HierarchyPanel::draw_entity_node(Entity entity) {
//     auto &nameComponent = entity.GetComponent<NameComponent>();
//     bool isSelected     = (m_selected_entity == entity);
//
//     ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth |
//                                (isSelected ? ImGuiTreeNodeFlags_Selected : 0);
//
//     bool isParent = entity.HasComponent<Hierarchy>() && entity.GetComponent<Hierarchy>().children.size() > 0;
//
//     if (!isParent)
//         flags |= ImGuiTreeNodeFlags_Leaf;
//
//     bool nodeOpen = ImGui::TreeNodeEx((void *) (uint64_t) entity.GetID(), flags, "%s", nameComponent.name.c_str());
//
//     // Handle selection
//     if (ImGui::IsItemClicked()) {
//         m_selected_entity = entity;
//     }
//
//     // Drag and Drop
//     if (ImGui::BeginDragDropSource()) {
//         ImGui::SetDragDropPayload("ENTITY_HIERARCHY", &entity, sizeof(Entity));
//         ImGui::Text("%s", nameComponent.name.c_str());
//         ImGui::EndDragDropSource();
//     }
//
//     // Handle dropping entities
//     draw_entity_drag_drop_target(entity);
//
//     // Context menu
//     if (ImGui::BeginPopupContextItem()) {
//         draw_entity_context_menu(entity);
//         ImGui::EndPopup();
//     }
//
//     // Recursive children drawing
//     if (nodeOpen) {
//         if (isParent) {
//             for (auto &child : entity.GetComponent<Hierarchy>().children) {
//                 draw_entity_node(child);
//             }
//         }
//         ImGui::TreePop();
//     }
// }
//
// void HierarchyPanel::draw_entity_context_menu(Entity entity) {
//     if (ImGui::MenuItem("Delete Entity")) {
//         if (auto scene = m_scene.lock()) {
//             scene->DestroyEntity(entity);
//             if (m_selected_entity == entity) {
//                 m_selected_entity = {};
//             }
//         }
//     }
//
//     if (ImGui::MenuItem("Create Child")) {
//         if (auto scene = m_scene.lock()) {
//             auto child = scene->CreateEntity("Child Entity");
//             child.SetParent(entity);
//         }
//     }
// }
//
// void HierarchyPanel::draw_entity_drag_drop_target(Entity parent) {
//     if (ImGui::BeginDragDropTarget()) {
//         if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ENTITY_HIERARCHY")) {
//             Entity droppedEntity = *static_cast<Entity *>(payload->Data);
//             if (auto scene = m_scene.lock()) {
//                 droppedEntity.set_parent(parent);
//             }
//         }
//         ImGui::EndDragDropTarget();
//     }
// }
