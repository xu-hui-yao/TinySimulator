#pragma once

#include <assets/resource_loader.h>
#include <assets/texture/texture.h>

class TextureLoader : public ResourceLoader {
public:
    TextureLoader() = default;

    ~TextureLoader() override = default;

    std::shared_ptr<Resource> load(const filesystem::path &path) override;

    bool save(std::shared_ptr<Resource> resource, const filesystem::path &path) override;

private:
    static std::shared_ptr<Texture> load_exr(const filesystem::path &path) noexcept;

    static std::shared_ptr<Texture> load_srgb(const filesystem::path &path) noexcept;

    static std::shared_ptr<Texture> load_hdr(const filesystem::path &path) noexcept;

    static bool save_exr(const std::shared_ptr<Texture>& texture, const filesystem::path &path) noexcept;

    static bool save_srgb(const std::shared_ptr<Texture>& texture, const filesystem::path &path) noexcept;

    static bool save_hdr(const std::shared_ptr<Texture>& texture, const filesystem::path &path) noexcept;

    static float srgb_to_linear(float c) noexcept;

    static float linear_to_srgb(float c) noexcept;
};