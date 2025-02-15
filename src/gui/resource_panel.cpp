// #include <core/fwd.h>
// #include <gui/resource_panel.h>
// #include <imgui.h>
// #include <scene/texture/texture_manager.h>
//
// ResourcePanel::ResourcePanel(const std::filesystem::path &working_directory) {
//     // Load icons
//     auto load_icon = [&](const char *path, file_type type) {
//         get_texture_manager()->load_resource(std::filesystem::path(path));
//         if (auto tex =
//                 std::dynamic_pointer_cast<Texture>(get_texture_manager()->get_resource(std::filesystem::path(path)))) {
//             m_icons[type] = tex;
//         }
//     };
//
//     load_icon("assets/icon/resource_panel/folder.png", file_type::e_folder);
//     load_icon("assets/icon/resource_panel/image.png", file_type::e_image);
//     load_icon("assets/icon/resource_panel/model.png", file_type::e_model);
//     load_icon("assets/icon/resource_panel/shader.png", file_type::e_shader);
//     load_icon("assets/icon/resource_panel/script.png", file_type::e_script);
//     load_icon("assets/icon/resource_panel/unknown.png", file_type::e_unknown);
//
//     set_current_dir(working_directory);
// }
//
// void ResourcePanel::on_ui_render() {
//     ImGui::Begin("Resource Browser");
//
//     // Navigation bar
//     ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
//     if (ImGui::Button("Refresh")) {
//         refresh();
//     }
//     ImGui::SameLine();
//     if (ImGui::Button("Back") && exists(m_current_dir.parent_path())) {
//         set_current_dir(m_current_dir.parent_path());
//     }
//     ImGui::SameLine();
//     ImGui::Text("Current: %s", m_current_dir.string().c_str());
//     ImGui::PopStyleVar();
//
//     // Split view
//     ImGui::BeginChild("##ResourcePanel", ImVec2(0, 0), true);
//     ImGui::Columns(2, "##resource_cols", true);
//
//     // Left panel: Directory tree
//     ImGui::BeginChild("##TreePane", ImVec2(200, 0), true);
//     if (m_need_refresh) {
//         refresh();
//         m_need_refresh = false;
//     }
//     for (auto &child : m_root.children) {
//         draw_tree_node(child);
//     }
//     ImGui::EndChild();
//
//     // Right panel: File list
//     ImGui::NextColumn();
//     ImGui::BeginChild("##FilePane", ImVec2(0, 0), true);
//     try {
//         for (const auto &entry : std::filesystem::directory_iterator(absolute(m_current_dir).string())) {
//             directory_info item;
//             item.path = entry.path();
//             item.type = entry.is_directory() ? file_type::e_folder
//                         : m_ext_map.contains(item.path.extension().string()) != 0
//                             ? m_ext_map[item.path.extension().string()]
//                             : file_type::e_unknown;
//
//             draw_file_item(item);
//         }
//     } catch (const std::exception &e) {
//         get_logger()->error("Directory error: {}", e.what());
//     }
//     ImGui::EndChild();
//
//     ImGui::EndChild();
//     ImGui::End();
// }
//
// void ResourcePanel::set_current_dir(const std::filesystem::path &path) {
//     if (exists(path) && is_directory(path)) {
//         m_current_dir  = path.lexically_normal();
//         m_need_refresh = true;
//     }
// }
//
// void ResourcePanel::refresh() {
//     m_root.children.clear();
//     m_root.path = get_file_resolver().resolve("assets").first;
//     build_tree(m_root);
// }
//
// void ResourcePanel::build_tree(directory_info &parent) {
//     try {
//         for (const auto &entry : std::filesystem::directory_iterator(parent.path)) {
//             if (entry.is_directory()) {
//                 directory_info child;
//                 child.path = entry.path();
//                 child.type = file_type::e_folder;
//                 parent.children.push_back(child);
//                 build_tree(parent.children.back());
//             }
//         }
//     } catch (const std::filesystem::filesystem_error &e) {
//         get_logger()->error("Directory error: {}", e.what());
//     }
// }
//
// void ResourcePanel::draw_tree_node(directory_info &node) {
//     ImGuiTreeNodeFlags flags  = (node.is_open ? ImGuiTreeNodeFlags_DefaultOpen : 0) | ImGuiTreeNodeFlags_OpenOnArrow |
//                                (node.children.empty() ? ImGuiTreeNodeFlags_Leaf : 0);
//
//     // 显示带图标的节点
//     ImGui::PushID(node.path.string().c_str());
//     bool is_selected = (node.path == m_current_dir);
//     if (is_selected) {
//         ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
//     }
//
//     // 使用图标+文字的方式显示
//     if (m_icons.contains(file_type::e_folder)) {
//         constexpr float icon_size = 16.0f;
//         ImGui::Image(m_icons[file_type::e_folder]->get_hash(), ImVec2(icon_size, icon_size));
//         ImGui::SameLine();
//     }
//
//     bool node_open = ImGui::TreeNodeEx(node.path.filename().string().c_str(), flags);
//
//     if (is_selected) {
//         ImGui::PopStyleColor();
//     }
//
//     // 处理点击事件
//     if (ImGui::IsItemClicked()) {
//         set_current_dir(node.path);
//     }
//
//     if (node_open) {
//         for (auto &child : node.children) {
//             draw_tree_node(child);
//         }
//         ImGui::TreePop();
//     }
//     ImGui::PopID();
// }
//
// void ResourcePanel::draw_file_item(const directory_info &item) {
//     constexpr float icon_size   = 24.0f;
//     // const ImVec4 selected_color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
//
//     ImGui::PushID(item.path.string().c_str());
//     ImGui::BeginGroup();
//
//     // 显示文件类型图标
//     if (const auto &icon = m_icons.contains(item.type) ? m_icons[item.type] : m_icons[file_type::e_unknown]) {
//         ImGui::Image(icon->get_hash(), ImVec2(icon_size, icon_size));
//     } else {
//         ImGui::Dummy(ImVec2(icon_size, icon_size));
//     }
//
//     ImGui::SameLine();
//     const std::string filename = item.path.filename().string();
//     ImGui::TextWrapped("%s", filename.c_str());
//
//     if (item.type == file_type::e_folder && ImGui::IsItemHovered() &&
//         ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
//         set_current_dir(item.path);
//     }
//
//     if (ImGui::BeginDragDropSource()) {
//         const auto &path_str = item.path.string();
//         ImGui::SetDragDropPayload("RESOURCE_PATH", path_str.c_str(), path_str.size() + 1);
//         ImGui::Text("Drag %s", filename.c_str());
//         ImGui::EndDragDropSource();
//     }
//
//     ImGui::EndGroup();
//     ImGui::PopID();
// }
