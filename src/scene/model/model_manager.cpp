#include <core/fwd.h>
#include <scene/model/model_manager.h>

ModelManager::ModelManager() : m_max_model_count(M_MAX_MODEL_COUNT), m_stop_hot_reload(false) {
    m_async_loader.start(M_MODEL_LOAD_THREAD);
}

ModelManager::~ModelManager() {
    if (m_hot_reload_thread.joinable()) {
        m_stop_hot_reload = true;
        m_hot_reload_thread.join();
    }
    m_async_loader.stop();
}

std::shared_ptr<Resource> ModelManager::load_resource(const std::filesystem::path &path,
                                                      const std::unordered_map<std::string, std::any> &param) {
    std::lock_guard lock(m_mutex);

    auto [resolved_path, exist] = get_file_resolver().resolve(path);
    if (!exist) {
        if (std::string(path).find(ModelLoader::internal_prefix) == 0) {
            auto resource = m_sync_loader.load(path, param);
            auto model    = std::dynamic_pointer_cast<Model>(resource);
            ModelRecord rec;
            rec.model         = model;
            rec.last_access   = std::chrono::system_clock::now();
            m_model_map[path] = rec;
            return resource;
        }
        get_logger()->error("Failed to load model {}", std::string(path));
        return nullptr;
    }
    auto canonical_path = canonical(resolved_path).string();
    auto it             = m_model_map.find(canonical_path);
    if (it != m_model_map.end()) {
        it->second.last_access = std::chrono::system_clock::now();
        return it->second.model;
    }

    auto resource = m_sync_loader.load(canonical_path, param);
    auto model    = std::dynamic_pointer_cast<Model>(resource);
    if (!model) {
        get_logger()->error("[ModelManager] Failed to load model: " + canonical_path);
        return nullptr;
    }

    ModelRecord rec;
    rec.model       = model;
    rec.last_access = std::chrono::system_clock::now();

    std::error_code ec;
    auto file_time = std::filesystem::last_write_time(absolute(path).string(), ec);
    if (!ec) {
        rec.last_write = file_time;
    }

    m_model_map[canonical_path] = rec;
    evict_if_needed();

    return model;
}

void ModelManager::load_resource_async(const std::filesystem::path &path,
                                       const std::unordered_map<std::string, std::any> &param) {
    auto [resolved_path, exist] = get_file_resolver().resolve(path);
    if (!exist) {
        if (std::string(path).find(ModelLoader::internal_prefix) == 0) {
            auto resource = m_sync_loader.load(path, param);
            auto model    = std::dynamic_pointer_cast<Model>(resource);
            ModelRecord rec;
            rec.model         = model;
            rec.last_access   = std::chrono::system_clock::now();
            m_model_map[path] = rec;
            return;
        }
        get_logger()->error("Failed to load model {}", std::string(path));
        return;
    }

    auto canonical_path = canonical(resolved_path).string();
    {
        std::lock_guard lock(m_mutex);
        if (m_model_map.contains(canonical_path)) {
            return;
        }
    }

    ResourceTask task;
    task.task_type = ResourceTaskType::Load;
    task.file_path = canonical_path;
    task.param = param;
    task.on_loaded = [this, canonical_path](const std::shared_ptr<Resource> &res) {
        if (!res) {
            get_logger()->error("[ModelManager] Async load returned null: " + canonical_path);
            return;
        }
        auto model = std::dynamic_pointer_cast<Model>(res);
        if (!model) {
            get_logger()->error("[ModelManager] Async load cast to Model failed: " + canonical_path);
            return;
        }

        std::lock_guard lk(m_mutex);
        ModelRecord rec;
        rec.model       = model;
        rec.last_access = std::chrono::system_clock::now();

        std::error_code ec;
        auto file_time = std::filesystem::last_write_time(canonical_path, ec);
        if (!ec) {
            rec.last_write = file_time;
        }

        m_model_map[canonical_path] = rec;
        evict_if_needed();
    };

    m_async_loader.enqueue_task(task);
}

void ModelManager::remove_resource(const std::filesystem::path &path) {
    std::lock_guard lock(m_mutex);
    auto [resolved_path, exist] = get_file_resolver().resolve(path);
    if (!exist) {
        get_logger()->error("ModelManger::load_resource - File " + resolved_path.string() + " not found.");
    }
    auto canonical_path = canonical(resolved_path).string();
    auto it             = m_model_map.find(canonical_path);
    if (it != m_model_map.end()) {
        m_model_map.erase(it);
    }
}

void ModelManager::remove_resource_async(const std::filesystem::path &path) {
    auto [resolved_path, exist] = get_file_resolver().resolve(path);
    if (!exist) {
        get_logger()->error("ModelManger::load_resource - File " + resolved_path.string() + " not found.");
    }
    auto canonical_path = canonical(resolved_path).string();

    ResourceTask task;
    task.task_type  = ResourceTaskType::Delete;
    task.file_path  = canonical_path;
    task.on_deleted = [this, canonical_path] {
        std::lock_guard lock(m_mutex);
        auto it = m_model_map.find(canonical_path);
        if (it != m_model_map.end()) {
            m_model_map.erase(it);
        }
    };

    m_async_loader.enqueue_task(task);
}

bool ModelManager::exist_resource(const std::filesystem::path &path) {
    std::lock_guard lock(m_mutex);
    auto [resolved_path, exist] = get_file_resolver().resolve(path);
    if (!exist) {
        get_logger()->error("ModelManger::load_resource - File " + resolved_path.string() + " not found.");
    }
    auto canonical_path = canonical(resolved_path).string();
    return m_model_map.contains(canonical_path);
}

std::shared_ptr<Resource> ModelManager::get_resource(const std::filesystem::path &path) {
    std::lock_guard lock(m_mutex);
    auto [resolved_path, exist] = get_file_resolver().resolve(path);
    if (!exist) {
        get_logger()->error("ModelManger::load_resource - File " + resolved_path.string() + " not found.");
    }
    auto canonical_path = canonical(resolved_path).string();
    auto it             = m_model_map.find(canonical_path);
    if (it == m_model_map.end()) {
        get_logger()->warn("[ModelManager] No such model: " + canonical_path);
        return nullptr;
    }
    it->second.last_access = std::chrono::system_clock::now();
    return it->second.model;
}

void ModelManager::enable_hot_reload(bool enable, std::chrono::seconds interval) {
    if (enable) {
        if (m_hot_reload_thread.joinable()) {
            return;
        }
        m_stop_hot_reload   = false;
        m_hot_reload_thread = std::thread(&ModelManager::hot_reload_thread_func, this, interval);
    } else {
        if (m_hot_reload_thread.joinable()) {
            m_stop_hot_reload = true;
            m_hot_reload_thread.join();
        }
    }
}

void ModelManager::hot_reload_thread_func(std::chrono::seconds interval) {
    while (!m_stop_hot_reload) {
        {
            std::lock_guard lock(m_mutex);
            for (auto it = m_model_map.begin(); it != m_model_map.end();) {
                const auto &filename = it->first;
                auto &record         = it->second;

                std::error_code ec;
                bool file_exists = std::filesystem::exists(filename, ec);
                if (ec || !file_exists) {
                    get_logger()->info("[ModelManager] Hot reload: file deleted => " + filename);
                    it = m_model_map.erase(it);
                    continue;
                }

                auto current_time = std::filesystem::last_write_time(filename, ec);
                if (!ec && current_time != record.last_write) {
                    get_logger()->info("[ModelManager] Hot reload triggered for " + filename);

                    ResourceTask task;
                    task.task_type    = ResourceTaskType::Reload;
                    task.file_path    = std::filesystem::path(filename);
                    task.old_resource = record.model;

                    task.on_loaded = [this, filename](const std::shared_ptr<Resource> &new_res) {
                        if (!new_res) {
                            get_logger()->error("[ModelManager] Hot reload failed: " + filename);
                            return;
                        }
                        auto new_model = std::dynamic_pointer_cast<Model>(new_res);
                        if (!new_model) {
                            get_logger()->error("[ModelManager] Hot reload cast failed: " + filename);
                            return;
                        }

                        std::lock_guard lk(m_mutex);
                        auto it2 = m_model_map.find(filename);
                        if (it2 != m_model_map.end()) {
                            it2->second.model       = new_model;
                            it2->second.last_access = std::chrono::system_clock::now();

                            std::error_code ec2;
                            auto file_time2 = std::filesystem::last_write_time(filename, ec2);
                            if (!ec2) {
                                it2->second.last_write = file_time2;
                            }
                        }
                    };

                    m_async_loader.enqueue_task(task);

                    record.last_write = current_time;
                }

                ++it;
            }
        }

        for (int i = 0; i < interval.count(); i++) {
            if (m_stop_hot_reload) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

void ModelManager::evict_if_needed() {
    if (m_model_map.size() <= m_max_model_count) {
        return;
    }

    auto oldest_it   = m_model_map.begin();
    auto oldest_time = oldest_it->second.last_access;

    for (auto it = std::next(m_model_map.begin()); it != m_model_map.end(); ++it) {
        if (it->second.last_access < oldest_time) {
            oldest_it   = it;
            oldest_time = it->second.last_access;
        }
    }

    get_logger()->info("[ModelManager] Evicting model: " + oldest_it->first);
    m_model_map.erase(oldest_it);
}

std::shared_ptr<ModelManager> get_model_manager() {
    static auto model_manager = std::make_shared<ModelManager>();
    return model_manager;
}
