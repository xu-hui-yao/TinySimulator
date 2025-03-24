#pragma once

#include <scene/model/async_model_loader.h>
#include <scene/model/model.h>
#include <scene/model/model_loader.h>
#include <scene/resource/resource_manager.h>
#include <thread>
#include <unordered_map>

class ModelManager : public ResourceManager {
public:
    ModelManager();

    ~ModelManager() override;

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
    struct ModelRecord {
        std::shared_ptr<Model> model;
        std::chrono::system_clock::time_point last_access;
        std::filesystem::file_time_type last_write;
    };

    std::unordered_map<std::string, ModelRecord> m_model_map;
    std::mutex m_mutex;

    size_t m_max_model_count;

    ModelLoader m_sync_loader;
    AsyncModelLoader m_async_loader;

    std::thread m_hot_reload_thread;
    std::atomic<bool> m_stop_hot_reload;

    void hot_reload_thread_func(std::chrono::seconds interval);

    void evict_if_needed();
};

std::shared_ptr<ModelManager> get_model_manager();
