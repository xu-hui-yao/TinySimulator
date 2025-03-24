#pragma once

#include <chrono>
#include <filesystem>
#include <mutex>
#include <scene/resource/resource_manager.h>
#include <scene/shader/shader.h>
#include <thread>
#include <unordered_map>

class ShaderManager : public ResourceManager {
public:
    ShaderManager();

    ~ShaderManager() override;

    std::shared_ptr<Resource> load_resource(const std::filesystem::path &path,
                                            const std::unordered_map<std::string, std::any> &param) override;

    void load_resource_async(const std::filesystem::path &path,
                             const std::unordered_map<std::string, std::any> &param) override;

    void remove_resource(const std::filesystem::path &path) override;

    void remove_resource_async(const std::filesystem::path &path) override;

    bool exist_resource(const std::filesystem::path &path) override;

    std::shared_ptr<Resource> get_resource(const std::filesystem::path &path) override;

    void enable_hot_reload(bool enable, std::chrono::seconds interval) override;

private:
    struct ShaderRecord {
        std::shared_ptr<Shader> shader;
        std::chrono::system_clock::time_point last_access;
        std::filesystem::file_time_type last_write;
    };

    std::unordered_map<std::string, ShaderRecord> m_shader_map;
    std::mutex m_mutex;
    size_t m_max_shader_count;
    std::thread m_hot_reload_thread;
    std::atomic<bool> m_stop_hot_reload;

    void hot_reload_thread_func(std::chrono::seconds interval);

    void evict_if_needed();
};

std::shared_ptr<ShaderManager> get_shader_manager();
