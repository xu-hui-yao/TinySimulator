#include <assets/texture/texture_manager.h>
#include <core/common.h>

TextureManager::TextureManager() : m_max_texture_count(M_MAX_TEXTURE_COUNT), m_stop_hot_reload(false) {
    m_async_loader.start(M_TEXTURE_LOAD_THREAD);
}

TextureManager::~TextureManager() {
    enable_hot_reload(false, std::chrono::seconds(M_HOT_RELOAD_SECONDS));
    m_async_loader.stop();
}

std::shared_ptr<Resource> TextureManager::load_resource(const filesystem::path &path) {
    std::lock_guard lock(m_mutex);

    auto abs_str = std::filesystem::absolute(path).string();
    auto it      = m_texture_map.find(abs_str);
    if (it != m_texture_map.end()) {
        // already loaded, update last access time
        it->second.last_access = std::chrono::system_clock::now();
        return it->second.texture;
    }

    // not loaded yet, do synchronous load
    auto res = m_sync_loader.load(path);
    if (!res) {
        throw std::runtime_error("[TextureManager] Failed to load texture: " + abs_str);
    }

    // store in map
    auto tex = std::dynamic_pointer_cast<Texture>(res);
    if (!tex) {
        throw std::runtime_error("[TextureManager] Resource is not a Texture: " + abs_str);
    }

    TextureRecord record;
    record.texture     = tex;
    record.last_access = std::chrono::system_clock::now();
    // record file write time
    std::error_code ec;
    auto ftime = std::filesystem::last_write_time(path, ec);
    if (!ec) {
        record.last_write = std::chrono::clock_cast<std::chrono::system_clock>(ftime);
    }
    m_texture_map[abs_str] = record;

    evict_if_needed();
    return tex;
}

void TextureManager::load_resource_async(const filesystem::path &path) {
    auto abs_path = std::filesystem::absolute(path).string();
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_texture_map.find(abs_path) != m_texture_map.end()) {
            // already exist
            std::cout << "[TextureManager] Texture already exists (async): " << abs_path << std::endl;
            return;
        }
    }

    // build a resource task
    ResourceTask task;
    task.task_type = ResourceTaskType::Load;
    task.file_path = path;

    // callback
    task.on_loaded = [this, abs_path](std::shared_ptr<Resource> res) {
        if (!res) {
            std::cerr << "[TextureManager] Async load returned null: " << abs_path << std::endl;
            return;
        }

        auto tex = std::dynamic_pointer_cast<Texture>(res);
        if (!tex) {
            std::cerr << "[TextureManager] Async load not a Texture: " << abs_path << std::endl;
            return;
        }

        std::lock_guard<std::mutex> lock(m_mutex);

        // store it
        TextureRecord record;
        record.texture     = tex;
        record.last_access = std::chrono::system_clock::now();

        std::error_code ec;
        auto ftime = std::filesystem::last_write_time(abs_path, ec);
        if (!ec) {
            record.last_write = std::chrono::clock_cast<std::chrono::system_clock>(ftime);
        }

        m_texture_map[abs_path] = record;
        evict_if_needed();
    };

    m_async_loader.enqueue_task(task);
}

void TextureManager::remove_resource(const filesystem::path &path) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto abs_str = std::filesystem::absolute(path).string();
    auto it      = m_texture_map.find(abs_str);
    if (it != m_texture_map.end()) {
        m_texture_map.erase(it);
        std::cout << "[TextureManager] Removed texture: " << abs_str << std::endl;
    } else {
        std::cout << "[TextureManager] Remove request for non-existing texture: " << abs_str << std::endl;
    }
}

void TextureManager::remove_resource_async(const filesystem::path &path) {
    auto abs_str = std::filesystem::absolute(path).string();

    // build a delete task
    ResourceTask task;
    task.task_type  = ResourceTaskType::Delete;
    task.file_path  = path;
    task.on_deleted = [this, abs_str]() {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_texture_map.find(abs_str);
        if (it != m_texture_map.end()) {
            m_texture_map.erase(it);
            std::cout << "[TextureManager] Async removed texture: " << abs_str << std::endl;
        }
    };

    m_async_loader.enqueue_task(task);
}

bool TextureManager::exist_resource(const filesystem::path &path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto abs_str = std::filesystem::absolute(path).string();
    return (m_texture_map.find(abs_str) != m_texture_map.end());
}

std::shared_ptr<Resource> TextureManager::get_resource(const filesystem::path &path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto abs_str = std::filesystem::absolute(path).string();
    auto it      = m_texture_map.find(abs_str);
    if (it == m_texture_map.end()) {
        throw std::runtime_error("[TextureManager] No such texture: " + abs_str);
    }
    // update last access
    it->second.last_access = std::chrono::system_clock::now();
    return it->second.texture;
}

void TextureManager::enable_hot_reload(bool enable, std::chrono::seconds interval) {
    if (enable) {
        if (m_hot_reload_thread.joinable()) {
            // already running
            return;
        }
        m_stop_hot_reload   = false;
        m_hot_reload_thread = std::thread(&TextureManager::hot_reload_thread_func, this, interval);
    } else {
        if (m_hot_reload_thread.joinable()) {
            m_stop_hot_reload = true;
            m_hot_reload_thread.join();
        }
    }
}

void TextureManager::hot_reload_thread_func(std::chrono::seconds interval) {
    while (!m_stop_hot_reload) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            for (auto it = m_texture_map.begin(); it != m_texture_map.end();) {
                const auto &filename = it->first;
                auto &record         = it->second;

                std::error_code ec;
                bool file_exists = std::filesystem::exists(filename, ec);
                if (ec || !file_exists) {
                    // file deleted => remove
                    std::cout << "[TextureManager] Hot reload: file deleted => " << filename << std::endl;
                    it = m_texture_map.erase(it); // erase returns next iterator
                    continue;                     // do not advance it
                }

                // check last write time
                auto current_time =
                    std::chrono::clock_cast<std::chrono::system_clock>(std::filesystem::last_write_time(filename, ec));
                if (!ec && current_time != record.last_write) {
                    // file changed => reload
                    std::cout << "[TextureManager] Hot reload triggered for " << filename << std::endl;

                    // build a reload task
                    ResourceTask task;
                    task.task_type    = ResourceTaskType::Reload;
                    task.file_path    = filename;
                    task.old_resource = record.texture; // if we want to do something with the old

                    task.on_loaded = [this, filename](std::shared_ptr<Resource> new_res) {
                        if (!new_res) {
                            std::cerr << "[TextureManager] Hot reload failed: " << filename << std::endl;
                            return;
                        }
                        auto new_tex = std::dynamic_pointer_cast<Texture>(new_res);
                        if (!new_tex) {
                            std::cerr << "[TextureManager] Hot reload not Texture: " << filename << std::endl;
                            return;
                        }

                        std::lock_guard<std::mutex> lk(m_mutex);
                        auto it2 = m_texture_map.find(filename);
                        if (it2 != m_texture_map.end()) {
                            // update record
                            it2->second.texture     = new_tex;
                            it2->second.last_access = std::chrono::system_clock::now();
                            std::error_code ec2;
                            auto ftime2 = std::filesystem::last_write_time(filename, ec2);
                            if (!ec2) {
                                it2->second.last_write = std::chrono::clock_cast<std::chrono::system_clock>(ftime2);
                            }
                            std::cout << "[TextureManager] Hot reloaded: " << filename << std::endl;
                        }
                    };

                    m_async_loader.enqueue_task(task);

                    // update old record time so we don't reload multiple times
                    record.last_write = current_time;
                }

                ++it;
            }
        }

        // sleep in smaller increments, so we can break early
        for (int i = 0; i < interval.count(); i++) {
            if (m_stop_hot_reload)
                break;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

void TextureManager::evict_if_needed() {
    if (m_texture_map.size() <= m_max_texture_count) {
        return;
    }

    // find least recently used
    auto oldest_it   = m_texture_map.begin();
    auto oldest_time = oldest_it->second.last_access;

    for (auto it = std::next(m_texture_map.begin()); it != m_texture_map.end(); ++it) {
        if (it->second.last_access < oldest_time) {
            oldest_it   = it;
            oldest_time = it->second.last_access;
        }
    }
    std::cout << "[TextureManager] Evicting texture: " << oldest_it->first << std::endl;
    m_texture_map.erase(oldest_it);
}
