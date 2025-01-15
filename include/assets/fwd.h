#pragma once

#include <memory>

class TextureLoader;
class AsyncTextureLoader;
class TextureManager;
class ModelLoader;
class AsyncModelLoader;
class ModelManager;

std::shared_ptr<TextureLoader> get_texture_loader();

std::shared_ptr<AsyncTextureLoader> get_async_texture_loader();

std::shared_ptr<TextureManager> get_texture_manager();

std::shared_ptr<ModelLoader> get_model_loader();

std::shared_ptr<AsyncModelLoader> get_async_model_loader();

std::shared_ptr<ModelManager> get_model_manager();
