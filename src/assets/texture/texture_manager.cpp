#include <assets/texture/texture_manager.h>
#include <iostream>
#include <stdexcept>

void TextureManager::add_texture(const std::string &path, TextureType texture_type) {
    std::lock_guard lock(texture_mutex);
    if (texture_map.find(path) != texture_map.end()) {
        std::cout << "Texture already exists: " + path << std::endl;
        return;
    }
    auto texture      = std::make_shared<Texture>(path, texture_type);
    texture_map[path] = texture;
}

void TextureManager::remove_texture(const std::string &path) {
    std::lock_guard lock(texture_mutex);
    auto it = texture_map.find(path);
    if (it != texture_map.end()) {
        texture_map.erase(it);
    } else {
        std::cout << "Texture name does not exist: " << path << std::endl;
    }
}

std::shared_ptr<Texture> TextureManager::get_texture(const std::string &path) {
    auto it = texture_map.find(path);
    if (it == texture_map.end()) {
        throw std::runtime_error("Texture name does not exist: " + path);
    }
    return it->second;
}