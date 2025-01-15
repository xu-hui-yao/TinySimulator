#include <assets/model/model_manager.h>
#include <core/common.h>

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

std::shared_ptr<Resource> ModelManager::load_resource(const filesystem::path &path) {
    std::lock_guard lock(m_mutex);

    auto abs_str = path.make_absolute().str();
    auto it      = m_model_map.find(abs_str);
    if (it != m_model_map.end()) {
        it->second.last_access = std::chrono::system_clock::now();
        return it->second.model;
    }

    auto resource = m_sync_loader.load(path);
    auto model    = std::dynamic_pointer_cast<Model>(resource);
    if (!model) {
        global::get_logger()->error("[ModelManager] Failed to load model: " + abs_str);
        return nullptr;
    }

    ModelRecord rec;
    rec.model       = model;
    rec.last_access = std::chrono::system_clock::now();

    std::error_code ec;
    auto file_time = std::filesystem::last_write_time(path.make_absolute().str(), ec);
    if (!ec) {
        rec.last_write = std::chrono::clock_cast<std::chrono::system_clock>(file_time);
    }

    m_model_map[abs_str] = rec;
    evict_if_needed();

    return model;
}

void ModelManager::load_resource_async(const filesystem::path &path) {
    auto abs_str = path.make_absolute().str();
    {
        std::lock_guard lock(m_mutex);
        if (m_model_map.contains(abs_str)) {
            return;
        }
    }

    // 构建异步任务
    ResourceTask task;
    task.task_type = ResourceTaskType::Load;
    task.file_path = path;
    task.on_loaded = [this, abs_str](const std::shared_ptr<Resource> &res) {
        if (!res) {
            global::get_logger()->error("[ModelManager] Async load returned null: " + abs_str);
            return;
        }
        auto model = std::dynamic_pointer_cast<Model>(res);
        if (!model) {
            global::get_logger()->error("[ModelManager] Async load cast to Model failed: " + abs_str);
            return;
        }

        std::lock_guard lk(m_mutex);
        ModelRecord rec;
        rec.model       = model;
        rec.last_access = std::chrono::system_clock::now();

        std::error_code ec;
        auto file_time = std::filesystem::last_write_time(abs_str, ec);
        if (!ec) {
            rec.last_write = std::chrono::clock_cast<std::chrono::system_clock>(file_time);
        }

        m_model_map[abs_str] = rec;
        evict_if_needed();
    };

    m_async_loader.enqueue_task(task);
}

void ModelManager::remove_resource(const filesystem::path &path) {
    std::lock_guard lock(m_mutex);
    auto abs_str = path.make_absolute().str();
    auto it      = m_model_map.find(abs_str);
    if (it != m_model_map.end()) {
        m_model_map.erase(it);
    }
}

void ModelManager::remove_resource_async(const filesystem::path &path) {
    auto abs_str = path.make_absolute().str();

    ResourceTask task;
    task.task_type  = ResourceTaskType::Delete;
    task.file_path  = path;
    task.on_deleted = [this, abs_str]() {
        std::lock_guard lock(m_mutex);
        auto it = m_model_map.find(abs_str);
        if (it != m_model_map.end()) {
            m_model_map.erase(it);
        }
    };

    m_async_loader.enqueue_task(task);
}

bool ModelManager::exist_resource(const filesystem::path &path) {
    std::lock_guard lock(m_mutex);
    return m_model_map.contains(path.make_absolute().str());
}

std::shared_ptr<Resource> ModelManager::get_resource(const filesystem::path &path) {
    std::lock_guard lock(m_mutex);
    auto abs_str = path.make_absolute().str();
    auto it      = m_model_map.find(abs_str);
    if (it == m_model_map.end()) {
        global::get_logger()->error("[ModelManager] No such model: " + abs_str);
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
                    global::get_logger()->info("[ModelManager] Hot reload: file deleted => " + filename);
                    it = m_model_map.erase(it);
                    continue;
                }

                auto current_time =
                    std::chrono::clock_cast<std::chrono::system_clock>(std::filesystem::last_write_time(filename, ec));
                if (!ec && current_time != record.last_write) {
                    global::get_logger()->info("[ModelManager] Hot reload triggered for " + filename);

                    ResourceTask task;
                    task.task_type    = ResourceTaskType::Reload;
                    task.file_path    = filesystem::path(filename);
                    task.old_resource = record.model;

                    task.on_loaded = [this, filename](const std::shared_ptr<Resource> &new_res) {
                        if (!new_res) {
                            global::get_logger()->error("[ModelManager] Hot reload failed: " + filename);
                            return;
                        }
                        auto new_model = std::dynamic_pointer_cast<Model>(new_res);
                        if (!new_model) {
                            global::get_logger()->error("[ModelManager] Hot reload cast failed: " + filename);
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
                                it2->second.last_write = std::chrono::clock_cast<std::chrono::system_clock>(file_time2);
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

    global::get_logger()->info("[ModelManager] Evicting model: " + oldest_it->first);
    m_model_map.erase(oldest_it);
}
