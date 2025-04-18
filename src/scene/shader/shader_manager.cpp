#include <core/fwd.h>
#include <filesystem>
#include <scene/shader/async_shader_loader.h>
#include <scene/shader/shader_loader.h>
#include <scene/shader/shader_manager.h>

ShaderManager::ShaderManager() : m_max_shader_count(M_MAX_SHADER_COUNT), m_stop_hot_reload(false) {
    get_async_shader_loader()->start(M_SHADER_LOAD_THREAD);
}

ShaderManager::~ShaderManager() {
    if (m_hot_reload_thread.joinable()) {
        m_stop_hot_reload = true;
        m_hot_reload_thread.join();
    }
    get_async_shader_loader()->stop();
}

std::shared_ptr<Resource> ShaderManager::load_resource(const std::filesystem::path &path,
                                                       const std::unordered_map<std::string, std::any> &param) {
    std::lock_guard lock(m_mutex);

    auto [resolved_path, exist] = get_file_resolver().resolve(path);
    if (!exist) {
        get_logger()->error("ShaderManger::load_resource - File " + resolved_path.string() + " not found.");
    }
    auto canonical_path = canonical(resolved_path).string();
    auto it             = m_shader_map.find(canonical_path);
    if (it != m_shader_map.end()) {
        it->second.last_access = std::chrono::system_clock::now();
        return it->second.shader;
    }

    // use sync loader
    auto resource = get_shader_loader()->load(canonical_path, param);
    auto shader   = std::dynamic_pointer_cast<Shader>(resource);
    if (!shader) {
        get_logger()->error("[ShaderManager] Failed to load shader: " + canonical_path);
        return nullptr;
    }

    // optional: immediately compile
    shader->upload(nullptr);

    ShaderRecord rec;
    rec.shader      = shader;
    rec.last_access = std::chrono::system_clock::now();

    std::error_code ec;
    auto file_time = std::filesystem::last_write_time(canonical_path, ec);
    if (!ec) {
        rec.last_write = file_time;
    }

    m_shader_map[canonical_path] = std::move(rec);
    evict_if_needed();

    return shader;
}

void ShaderManager::load_resource_async(const std::filesystem::path &path,
                                        const std::unordered_map<std::string, std::any> &param) {
    auto [resolved_path, exist] = get_file_resolver().resolve(path);
    if (!exist) {
        get_logger()->error("ShaderManger::load_resource - File " + resolved_path.string() + " not found.");
    }
    auto canonical_path = canonical(resolved_path).string();
    {
        std::lock_guard lock(m_mutex);
        if (m_shader_map.contains(canonical_path)) {
            return;
        }
    }

    ResourceTask task;
    task.task_type = ResourceTaskType::Load;
    task.file_path = canonical_path;
    task.param = param;
    task.on_loaded = [this, canonical_path](const std::shared_ptr<Resource> &res) {
        if (!res) {
            get_logger()->error("[ShaderManager] Async load returned null: " + canonical_path);
            return;
        }
        auto shader = std::dynamic_pointer_cast<Shader>(res);
        if (!shader) {
            get_logger()->error("[ShaderManager] Async load cast to Shader failed: " + canonical_path);
            return;
        }

        shader->upload(nullptr);

        std::lock_guard lk(m_mutex);
        ShaderRecord rec;
        rec.shader      = shader;
        rec.last_access = std::chrono::system_clock::now();

        std::error_code ec;
        auto file_time = std::filesystem::last_write_time(canonical_path, ec);
        if (!ec) {
            rec.last_write = file_time;
        }

        m_shader_map[canonical_path] = std::move(rec);
        evict_if_needed();
    };

    get_async_shader_loader()->enqueue_task(task);
}

void ShaderManager::remove_resource(const std::filesystem::path &path) {
    std::lock_guard lock(m_mutex);
    auto [resolved_path, exist] = get_file_resolver().resolve(path);
    if (!exist) {
        get_logger()->error("ShaderManger::load_resource - File " + resolved_path.string() + " not found.");
    }
    auto canonical_path = canonical(resolved_path).string();
    auto it             = m_shader_map.find(canonical_path);
    if (it != m_shader_map.end()) {
        m_shader_map.erase(it);
    }
}

void ShaderManager::remove_resource_async(const std::filesystem::path &path) {
    auto [resolved_path, exist] = get_file_resolver().resolve(path);
    if (!exist) {
        get_logger()->error("ShaderManger::load_resource - File " + resolved_path.string() + " not found.");
    }
    auto canonical_path = canonical(resolved_path).string();

    ResourceTask task;
    task.task_type  = ResourceTaskType::Delete;
    task.file_path  = canonical_path;
    task.on_deleted = [this, canonical_path] {
        std::lock_guard lock(m_mutex);
        auto it = m_shader_map.find(canonical_path);
        if (it != m_shader_map.end()) {
            m_shader_map.erase(it);
        }
    };

    get_async_shader_loader()->enqueue_task(task);
}

bool ShaderManager::exist_resource(const std::filesystem::path &path) {
    std::lock_guard lock(m_mutex);
    auto [resolved_path, exist] = get_file_resolver().resolve(path);
    if (!exist) {
        get_logger()->error("ShaderManger::load_resource - File " + resolved_path.string() + " not found.");
    }
    auto canonical_path = canonical(resolved_path).string();
    return m_shader_map.contains(canonical_path);
}

std::shared_ptr<Resource> ShaderManager::get_resource(const std::filesystem::path &path) {
    std::lock_guard lock(m_mutex);
    auto [resolved_path, exist] = get_file_resolver().resolve(path);
    if (!exist) {
        get_logger()->error("ShaderManger::load_resource - File " + resolved_path.string() + " not found.");
    }
    auto canonical_path = canonical(resolved_path).string();
    auto it             = m_shader_map.find(canonical_path);
    if (it == m_shader_map.end()) {
        // get_logger()->error("[ShaderManager] No such shader: " + abs_str);
        return nullptr;
    }
    it->second.last_access = std::chrono::system_clock::now();
    return it->second.shader;
}

void ShaderManager::enable_hot_reload(bool enable, std::chrono::seconds interval) {
    if (enable) {
        if (m_hot_reload_thread.joinable()) {
            return;
        }
        m_stop_hot_reload   = false;
        m_hot_reload_thread = std::thread(&ShaderManager::hot_reload_thread_func, this, interval);
    } else {
        if (m_hot_reload_thread.joinable()) {
            m_stop_hot_reload = true;
            m_hot_reload_thread.join();
        }
    }
}

void ShaderManager::hot_reload_thread_func(std::chrono::seconds interval) {
    while (!m_stop_hot_reload) {
        {
            std::lock_guard lock(m_mutex);
            for (auto it = m_shader_map.begin(); it != m_shader_map.end();) {
                const auto &filename = it->first;
                auto &record         = it->second;

                std::error_code ec;
                bool file_exists = std::filesystem::exists(filename, ec);
                if (ec || !file_exists) {
                    get_logger()->info("[ShaderManager] Hot reload: file deleted => " + filename);
                    it = m_shader_map.erase(it);
                    continue;
                }

                auto current_time = std::filesystem::last_write_time(filename, ec);
                if (!ec && current_time != record.last_write) {
                    get_logger()->info("[ShaderManager] Hot reload triggered for " + filename);

                    ResourceTask task;
                    task.task_type    = ResourceTaskType::Reload;
                    task.file_path    = std::filesystem::path(filename);
                    task.old_resource = record.shader;
                    task.on_loaded    = [this, filename](const std::shared_ptr<Resource> &new_res) {
                        if (!new_res) {
                            get_logger()->error("[ShaderManager] Hot reload failed: " + filename);
                            return;
                        }
                        auto new_shader = std::dynamic_pointer_cast<Shader>(new_res);
                        if (!new_shader) {
                            get_logger()->error("[ShaderManager] Hot reload cast failed: " + filename);
                            return;
                        }

                        // compile
                        new_shader->upload(nullptr);

                        std::lock_guard lk(m_mutex);
                        auto it2 = m_shader_map.find(filename);
                        if (it2 != m_shader_map.end()) {
                            it2->second.shader      = new_shader;
                            it2->second.last_access = std::chrono::system_clock::now();

                            std::error_code ec2;
                            auto file_time2 = std::filesystem::last_write_time(filename, ec2);
                            if (!ec2) {
                                it2->second.last_write = file_time2;
                            }
                        }
                    };

                    get_async_shader_loader()->enqueue_task(task);
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

void ShaderManager::evict_if_needed() {
    if (m_shader_map.size() <= m_max_shader_count) {
        return;
    }

    auto oldest_it   = m_shader_map.begin();
    auto oldest_time = oldest_it->second.last_access;
    for (auto it = std::next(m_shader_map.begin()); it != m_shader_map.end(); ++it) {
        if (it->second.last_access < oldest_time) {
            oldest_it   = it;
            oldest_time = it->second.last_access;
        }
    }

    get_logger()->info("[ShaderManager] Evicting shader: " + oldest_it->first);
    m_shader_map.erase(oldest_it);
}

std::shared_ptr<ShaderManager> get_shader_manager() {
    static auto shader_manager = std::make_shared<ShaderManager>();
    return shader_manager;
}
