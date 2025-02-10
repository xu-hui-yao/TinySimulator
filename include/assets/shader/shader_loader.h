#pragma once

#include <assets/resource/resource_loader.h>
#include <assets/shader/shader.h>

class ShaderLoader : public ResourceLoader {
public:
    ShaderLoader() = default;

    ~ShaderLoader() override = default;

    std::shared_ptr<Resource> load(const filesystem::path &path) override;

    bool save(std::shared_ptr<Resource> resource, const filesystem::path &path) override;
};
