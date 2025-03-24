#pragma once

#include <scene/camera/camera.h>
#include <scene/light/light.h>
#include <scene/model/model.h>
#include <scene/shader/shader.h>
#include <unordered_map>
#include <any>

class Scene {
public:
    void create_child(const std::string &name, const std::shared_ptr<Scene> &scene);

    std::shared_ptr<Scene> get_child(const std::string &name);

    std::shared_ptr<Scene> get_parent();

    void load_model(const std::string &name, const std::filesystem::path &path,
                    const std::unordered_map<std::string, std::any> &param = {});

    std::shared_ptr<Model> get_model(const std::string &name);

    void load_shader(const std::string &name, const std::filesystem::path &path,
                     const std::unordered_map<std::string, std::any> &param = {});

    std::shared_ptr<Shader> get_shader(const std::string &name);

    void set_current_shader(const std::string &name);

    std::shared_ptr<Shader> get_current_shader();

    void set_camera(const std::string &name, const std::shared_ptr<Camera> &camera);

    std::shared_ptr<Camera> get_camera(const std::string &name);

    void set_main_camera(const std::string &name);

    std::shared_ptr<Camera> get_main_camera();

    void set_light(const std::string &name, const std::shared_ptr<Light> &light);

    std::shared_ptr<Light> get_light(const std::string &name);

    void set_shadow_light(const std::string &name);

private:
    std::shared_ptr<Scene> m_parent{ nullptr };
    std::shared_ptr<Camera> m_main_camera{ nullptr };
    std::shared_ptr<Light> m_shadow_light{ nullptr };
    std::shared_ptr<Shader> m_current_shader{ nullptr };
    std::unordered_map<std::string, std::shared_ptr<Scene>> m_children;
    std::unordered_map<std::string, std::shared_ptr<Model>> m_models;
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
    std::unordered_map<std::string, std::shared_ptr<Camera>> m_cameras;
    std::unordered_map<std::string, std::shared_ptr<Light>> m_lights;
};

std::shared_ptr<Scene> get_root_scene();
