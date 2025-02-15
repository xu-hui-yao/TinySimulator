#include <scene/shader/shader_loader.h>
#include <core/fwd.h>

std::shared_ptr<Resource> ShaderLoader::load(const std::filesystem::path &path) {
    return std::make_shared<Shader>(path);
}

bool ShaderLoader::save(std::shared_ptr<Resource> /*resource*/, const std::filesystem::path & /*path*/) {
    get_logger()->warn("[ShaderLoader] save not implemented");
    return false;
}

std::shared_ptr<ShaderLoader> get_shader_loader() {
    static auto shader_loader = std::make_shared<ShaderLoader>();
    return shader_loader;
}
