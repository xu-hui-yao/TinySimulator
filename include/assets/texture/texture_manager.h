#pragma once

#include <assets/resource_manager.h>
#include <assets/texture/async_texture_loader.h>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <thread>
#include <atomic>

/**
 * @brief A concrete ResourceManager that manages Texture resources.
 */
class TextureManager : public ResourceManager {
public:
    TextureManager();

    ~TextureManager() override;

    std::shared_ptr<Resource> load_resource(const filesystem::path &path) override;

    void load_resource_async(const filesystem::path &path) override;

    void remove_resource(const filesystem::path &path) override;

    void remove_resource_async(const filesystem::path &path) override;

    bool exist_resource(const filesystem::path &path) override;

    std::shared_ptr<Resource> get_resource(const filesystem::path &path) override;

    void enable_hot_reload(bool enable, std::chrono::seconds interval) override;

private:
    struct TextureRecord {
        std::shared_ptr<Texture> texture;
        std::chrono::system_clock::time_point last_access;
        std::chrono::system_clock::time_point last_write;
    };

    std::unordered_map<std::string, TextureRecord> m_texture_map;
    std::mutex m_mutex;
    size_t m_max_texture_count;
    TextureLoader m_sync_loader;
    AsyncTextureLoader m_async_loader;
    std::thread m_hot_reload_thread;
    std::atomic<bool> m_stop_hot_reload;

    void hot_reload_thread_func(std::chrono::seconds interval);

    void evict_if_needed();
};
