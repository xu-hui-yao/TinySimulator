#include <assets/fwd.h>
#include <assets/model/async_model_loader.h>
#include <assets/model/model_loader.h>
#include <assets/model/model_manager.h>
#include <assets/texture/async_texture_loader.h>
#include <assets/texture/texture_loader.h>
#include <assets/texture/texture_manager.h>

std::shared_ptr<TextureLoader> get_texture_loader() {
    static auto texture_loader = std::make_shared<TextureLoader>();
    return texture_loader;
}

std::shared_ptr<AsyncTextureLoader> get_async_texture_loader() {
    static auto async_texture_loader = std::make_shared<AsyncTextureLoader>();
    return async_texture_loader;
}

std::shared_ptr<TextureManager> get_texture_manager() {
    static auto texture_manager = std::make_shared<TextureManager>();
    return texture_manager;
}

std::shared_ptr<ModelLoader> get_model_loader() {
    static auto model_loader = std::make_shared<ModelLoader>();
    return model_loader;
}

std::shared_ptr<AsyncModelLoader> get_async_model_loader() {
    static auto async_model_loader = std::make_shared<AsyncModelLoader>();
    return async_model_loader;
}

std::shared_ptr<ModelManager> get_model_manager() {
    static auto model_manager = std::make_shared<ModelManager>();
    return model_manager;
}