#include <scene/model/model_manager.h>
#include <scene/scene/scene.h>
#include <scene/shader/shader_manager.h>

void Scene::create_child(const std::string &name, const std::shared_ptr<Scene> &scene) {
    if (m_children.contains(name)) {
        get_logger()->error("Scene::create_child: name already exists");
        return;
    }

    m_children[name] = scene;
}

std::shared_ptr<Scene> Scene::get_child(const std::string &name) {
    if (m_children.contains(name)) {
        return m_children[name];
    }

    get_logger()->error("Scene::get_child: name not found");
    return nullptr;
}

std::shared_ptr<Scene> Scene::get_parent() { return m_parent; }

void Scene::load_model(const std::string &name, const std::filesystem::path &path) {
    if (m_models.contains(name)) {
        get_logger()->error("Scene::load_model: name already exists");
        return;
    }

    auto model     = get_model_manager()->load_resource(path);
    m_models[name] = std::dynamic_pointer_cast<Model>(model);
}

std::shared_ptr<Model> Scene::get_model(const std::string &name) {
    if (m_models.contains(name)) {
        return m_models[name];
    }

    get_logger()->error("Scene::get_model: name not found");
    return nullptr;
}

void Scene::load_shader(const std::string &name, const std::filesystem::path &path) {
    if (m_shaders.contains(name)) {
        get_logger()->error("Scene::load_shader: name already exists");
        return;
    }

    auto shader     = get_shader_manager()->load_resource(path);
    m_shaders[name] = std::dynamic_pointer_cast<Shader>(shader);
}

std::shared_ptr<Shader> Scene::get_shader(const std::string &name) {
    if (m_shaders.contains(name)) {
        return m_shaders[name];
    }

    get_logger()->error("Scene::get_shader: name not found");
    return nullptr;
}

void Scene::set_camera(const std::string &name, const std::shared_ptr<Camera> &camera) {
    if (m_cameras.contains(name)) {
        get_logger()->error("Scene::set_camera: name already exists");
        return;
    }

    m_cameras[name] = camera;
}

std::shared_ptr<Camera> Scene::get_camera(const std::string &name) {
    if (m_cameras.contains("camera")) {
        return m_cameras["camera"];
    }
    get_logger()->error("Scene::get_camera: name not found");
    return nullptr;
}

void Scene::set_main_camera(const std::string &name) {
    if (m_cameras.contains(name)) {
        m_main_camera = m_cameras[name];
    }

    get_logger()->error("Scene::set_main_camera: name not exists");
}

void Scene::set_light(const std::string &name, const std::shared_ptr<Light> &light) {
    if (m_lights.contains(name)) {
        get_logger()->error("Scene::set_light: name already exists");
        return;
    }

    m_lights[name] = light;
}

std::shared_ptr<Light> Scene::get_light(const std::string &name) {
    if (m_lights.contains(name)) {
        return m_lights[name];
    }

    get_logger()->error("Scene::get_light: name not found");
    return nullptr;
}

void Scene::set_shadow_light(const std::string &name) {
    if (m_lights.contains(name)) {
        m_shadow_light = m_lights[name];
    }

    get_logger()->error("Scene::set_shadow_light: name not found");
}