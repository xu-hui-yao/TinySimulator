#include <assets/shader/shader_loader.h>
#include <core/common.h>

std::shared_ptr<Resource> ShaderLoader::load(const filesystem::path &path) {
    return std::make_shared<Shader>(path);
}

bool ShaderLoader::save(std::shared_ptr<Resource> /*resource*/, const filesystem::path & /*path*/) {
    global::get_logger()->warn("[ShaderLoader] save not implemented");
    return false;
}
