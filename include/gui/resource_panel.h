// #pragma once
//
// #include <filesystem>
// #include <scene/texture/texture.h>
// #include <unordered_map>
//
// // File type categorization
// enum class file_type { e_unknown = 0, e_folder, e_image, e_model, e_shader, e_script };
//
// // Structure to hold directory information
// struct directory_info {
//     std::filesystem::path path;
//     file_type type;
//     bool is_open     = false;
//     bool is_selected = false;
//     std::vector<directory_info> children;
// };
//
// class ResourcePanel {
// public:
//     explicit ResourcePanel(const std::filesystem::path &working_directory);
//     void on_ui_render();
//     void set_current_dir(const std::filesystem::path &path);
//
// private:
//     void refresh();
//     static void build_tree(directory_info &parent);
//     void draw_tree_node(directory_info &node);
//     void draw_file_item(const directory_info &item);
//
//     std::filesystem::path m_current_dir;
//     directory_info m_root;
//     bool m_need_refresh = true;
//
//     // Icon textures
//     std::unordered_map<file_type, std::shared_ptr<Texture>> m_icons;
//     std::unordered_map<std::string, file_type> m_ext_map = {
//         { ".png", file_type::e_image },   { ".jpg", file_type::e_image },   { ".jpeg", file_type::e_image },
//         { ".obj", file_type::e_model },   { ".gltf", file_type::e_model },  { ".fbx", file_type::e_model },
//         { ".glsl", file_type::e_shader }, { ".vert", file_type::e_shader }, { ".frag", file_type::e_shader },
//         { ".lua", file_type::e_script },  { ".txt", file_type::e_script }
//     };
// };
