#pragma once

#include <scene/resource/resource_loader.h>
#include <scene/shader/shader.h>

class ShaderLoader : public ResourceLoader {
public:
    ShaderLoader() = default;

    ~ShaderLoader() override = default;

    std::shared_ptr<Resource> load(const std::filesystem::path &path,
                                   const std::unordered_map<std::string, std::any> &param) override;

    bool save(std::shared_ptr<Resource> resource, const std::filesystem::path &path) override;
};

std::shared_ptr<ShaderLoader> get_shader_loader();
