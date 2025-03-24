#pragma once

#include <filesystem>
#include <scene/resource/resource_loader.h>
#include <scene/texture/texture.h>

class TextureLoader : public ResourceLoader {
public:
    TextureLoader() = default;

    ~TextureLoader() override = default;

    std::shared_ptr<Resource> load(const std::filesystem::path &path,
                                   const std::unordered_map<std::string, std::any> &param) override;

    bool save(std::shared_ptr<Resource> resource, const std::filesystem::path &path) override;

private:
    static std::shared_ptr<Texture> load_exr(const std::filesystem::path &path) noexcept;

    static std::shared_ptr<Texture> load_srgb(const std::filesystem::path &path) noexcept;

    static std::shared_ptr<Texture> load_hdr(const std::filesystem::path &path) noexcept;

    static bool save_exr(const std::shared_ptr<Texture> &texture, const std::filesystem::path &path) noexcept;

    static bool save_srgb(const std::shared_ptr<Texture> &texture, const std::filesystem::path &path) noexcept;

    static bool save_hdr(const std::shared_ptr<Texture> &texture, const std::filesystem::path &path) noexcept;
};

std::shared_ptr<TextureLoader> get_texture_loader();
