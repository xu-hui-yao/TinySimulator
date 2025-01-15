#pragma once

#include <assets/resource_manager.h>
#include <assets/shader/shader.h>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>

class ShaderManager : public ResourceManager {
public:
    ShaderManager();

    ~ShaderManager() override;

    std::shared_ptr<Resource> load_resource(const filesystem::path &path) override;

    void load_resource_async(const filesystem::path &path) override;

    void remove_resource(const filesystem::path &path) override;

    void remove_resource_async(const filesystem::path &path) override;

    bool exist_resource(const filesystem::path &path) override;

    std::shared_ptr<Resource> get_resource(const filesystem::path &path) override;

    void enable_hot_reload(bool enable, std::chrono::seconds interval) override;

private:
    struct ShaderRecord {
        std::shared_ptr<Shader> shader;
        std::chrono::system_clock::time_point last_access;
        std::chrono::system_clock::time_point last_write;
    };

    std::unordered_map<std::string, ShaderRecord> m_shader_map;
    std::mutex m_mutex;
    size_t m_max_shader_count;
    std::thread m_hot_reload_thread;
    std::atomic<bool> m_stop_hot_reload;

    void hot_reload_thread_func(std::chrono::seconds interval);

    void evict_if_needed();
};
