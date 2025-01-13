#pragma once

#include <assets/texture/texture.h>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

class TextureManager {
public:
    TextureManager() = default;

    ~TextureManager() = default;

    void add_texture(const std::string &path, TextureType texture_type);

    void remove_texture(const std::string &path);

    std::shared_ptr<Texture> get_texture(const std::string &path);

private:
    std::unordered_map<std::string, std::shared_ptr<Texture>> texture_map;
    std::mutex texture_mutex;
};
