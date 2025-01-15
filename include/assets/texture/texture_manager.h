#pragma once

#include <assets/resource_manager.h>
#include <assets/texture/async_texture_loader.h>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <unordered_map>

/**
 * @brief A concrete ResourceManager that manages Texture resources.
 */
class TextureManager : public ResourceManager {
public:
    TextureManager() noexcept;

    ~TextureManager() noexcept override;

    std::shared_ptr<Resource> load_resource(const filesystem::path &path) noexcept override;

    void load_resource_async(const filesystem::path &path) noexcept override;

    void remove_resource(const filesystem::path &path) noexcept override;

    void remove_resource_async(const filesystem::path &path) noexcept override;

    bool exist_resource(const filesystem::path &path) noexcept override;

    std::shared_ptr<Resource> get_resource(const filesystem::path &path) noexcept override;

    void enable_hot_reload(bool enable, std::chrono::seconds interval) noexcept override;

private:
    struct TextureRecord {
        std::shared_ptr<Texture> texture;
        std::chrono::system_clock::time_point last_access;
        std::chrono::system_clock::time_point last_write;
    };

    std::unordered_map<std::string, TextureRecord> m_texture_map;
    std::mutex m_mutex;
    size_t m_max_texture_count;
    std::thread m_hot_reload_thread;
    std::atomic<bool> m_stop_hot_reload;

    void hot_reload_thread_func(std::chrono::seconds interval) noexcept;

    void evict_if_needed() noexcept;
};
