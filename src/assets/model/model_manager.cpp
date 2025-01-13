#include <assets/model/model_manager.h>
#include <iostream>

ModelManager::ModelManager(const std::shared_ptr<TextureManager> &texture_manager) : texture_manager(texture_manager) {}

void ModelManager::add_model(const std::string &path) {
    std::lock_guard lock(model_mutex);
    if (model_map.find(path) != model_map.end()) {
        std::cout << "Model already exists: " + path << std::endl;
        return;
    }
    auto model      = std::make_shared<Model>(path);
    model_map[path] = model;
}

void ModelManager::remove_model(const std::string &path) {
    std::lock_guard lock(model_mutex);
    auto it = model_map.find(path);
    if (it != model_map.end()) {
        model_map.erase(it);
    } else {
        std::cout << "Model name does not exist: " << path << std::endl;
    }
}

std::shared_ptr<Model> ModelManager::get_model(const std::string &path) {
    auto it = model_map.find(path);
    if (it == model_map.end()) {
        throw std::runtime_error("Model name does not exist: " + path);
    }
    return it->second;
}