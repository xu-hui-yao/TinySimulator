#include <assets/fwd.h>
#include <assets/texture/texture_manager.h>
#include <core/global.h>
#include <filesystem>

TextureManager::TextureManager() noexcept : m_max_texture_count(M_MAX_TEXTURE_COUNT), m_stop_hot_reload(false) {
    get_async_texture_loader()->start(M_TEXTURE_LOAD_THREAD);
}

TextureManager::~TextureManager() noexcept {
    if (m_hot_reload_thread.joinable()) {
        m_stop_hot_reload = true;
        m_hot_reload_thread.join();
    }
    get_async_texture_loader()->stop();
}

std::shared_ptr<Resource> TextureManager::load_resource(const filesystem::path &path) noexcept {
    std::lock_guard lock(m_mutex);

    auto abs_str = path.make_absolute().str();
    auto it      = m_texture_map.find(abs_str);
    if (it != m_texture_map.end()) {
        // already loaded, update last access time
        it->second.last_access = std::chrono::system_clock::now();
        return it->second.texture;
    }

    // not loaded yet, do synchronous load
    auto texture = std::dynamic_pointer_cast<Texture>(get_texture_loader()->load(path));
    if (!texture) {
        get_logger()->error("[TextureManager] Failed to load texture: " + abs_str);
        return nullptr;
    }

    TextureRecord record;
    record.texture     = texture;
    record.last_access = std::chrono::system_clock::now();
    std::error_code ec;
    auto file_time = std::filesystem::last_write_time(path.make_absolute().str(), ec);
    if (!ec) {
        record.last_write = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            file_time - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    }
    m_texture_map[abs_str] = record;

    evict_if_needed();
    return texture;
}

void TextureManager::load_resource_async(const filesystem::path &path) noexcept {
    auto abs_path = path.make_absolute().str();
    {
        std::lock_guard lock(m_mutex);
        if (m_texture_map.contains(abs_path)) {
            return;
        }
    }

    // build a resource task
    ResourceTask task;
    task.task_type = ResourceTaskType::Load;
    task.file_path = path;

    // callback
    task.on_loaded = [this, abs_path](const std::shared_ptr<Resource> &resource) {
        if (!resource) {
            get_logger()->error("[TextureManager] Async load returned null: " + abs_path);
            return;
        }
        auto texture = std::dynamic_pointer_cast<Texture>(resource);
        std::lock_guard lock(m_mutex);

        // store it
        TextureRecord record;
        record.texture     = texture;
        record.last_access = std::chrono::system_clock::now();

        std::error_code ec;
        auto file_time = std::filesystem::last_write_time(abs_path, ec);
        if (!ec) {
            record.last_write = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                file_time - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
        }

        m_texture_map[abs_path] = record;
        evict_if_needed();
    };

    get_async_texture_loader()->enqueue_task(task);
}

void TextureManager::remove_resource(const filesystem::path &path) noexcept {
    std::lock_guard lock(m_mutex);

    auto abs_str = path.make_absolute().str();
    auto it      = m_texture_map.find(abs_str);
    if (it != m_texture_map.end()) {
        m_texture_map.erase(it);
    }
}

void TextureManager::remove_resource_async(const filesystem::path &path) noexcept {
    auto abs_str = path.make_absolute().str();

    // build a delete task
    ResourceTask task;
    task.task_type  = ResourceTaskType::Delete;
    task.file_path  = path;
    task.on_deleted = [this, abs_str]() {
        std::lock_guard lock(m_mutex);
        auto it = m_texture_map.find(abs_str);
        if (it != m_texture_map.end()) {
            m_texture_map.erase(it);
        }
    };

    get_async_texture_loader()->enqueue_task(task);
}

bool TextureManager::exist_resource(const filesystem::path &path) noexcept {
    std::lock_guard lock(m_mutex);
    auto abs_str = path.make_absolute().str();
    return m_texture_map.contains(abs_str);
}

std::shared_ptr<Resource> TextureManager::get_resource(const filesystem::path &path) noexcept {
    std::lock_guard lock(m_mutex);
    auto abs_str = path.make_absolute().str();
    auto it      = m_texture_map.find(abs_str);
    if (it == m_texture_map.end()) {
        get_logger()->error("[TextureManager] No such texture: " + abs_str);
        return nullptr;
    }
    it->second.last_access = std::chrono::system_clock::now();
    return it->second.texture;
}

void TextureManager::enable_hot_reload(bool enable, std::chrono::seconds interval) noexcept {
    if (enable) {
        if (m_hot_reload_thread.joinable()) { // already running
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

void TextureManager::hot_reload_thread_func(std::chrono::seconds interval) noexcept {
    while (!m_stop_hot_reload) {
        {
            std::lock_guard lock(m_mutex);

            for (auto it = m_texture_map.begin(); it != m_texture_map.end();) {
                const auto &filename = it->first;
                auto &record         = it->second;

                std::error_code ec;
                bool file_exists = std::filesystem::exists(filename, ec);
                if (ec || !file_exists) {
                    // file deleted => remove
                    get_logger()->info("[TextureManager] Hot reload: file deleted => " + filename);
                    it = m_texture_map.erase(it); // erase returns next iterator
                    continue;                     // do not advance it
                }

                // check last write time
                auto current_time =
                    std::chrono::clock_cast<std::chrono::system_clock>(std::filesystem::last_write_time(filename, ec));
                if (!ec && current_time != record.last_write) {
                    // file changed => reload
                    get_logger()->info("[TextureManager] Hot reload triggered for " + filename);

                    // build a reload task
                    ResourceTask task;
                    task.task_type    = ResourceTaskType::Reload;
                    task.file_path    = filesystem::path(filename);
                    task.old_resource = record.texture; // if we want to do something with the old

                    task.on_loaded = [this, filename](const std::shared_ptr<Resource> &new_resource) {
                        if (!new_resource) {
                            get_logger()->error("[TextureManager] Hot reload failed: " + filename);
                            return;
                        }
                        auto new_tex = std::dynamic_pointer_cast<Texture>(new_resource);

                        std::lock_guard lk(m_mutex);
                        auto it2 = m_texture_map.find(filename);
                        if (it2 != m_texture_map.end()) {
                            // update record
                            it2->second.texture     = new_tex;
                            it2->second.last_access = std::chrono::system_clock::now();
                            std::error_code ec2;
                            auto file_time2 = std::filesystem::last_write_time(filename, ec2);
                            if (!ec2) {
                                it2->second.last_write = std::chrono::clock_cast<std::chrono::system_clock>(file_time2);
                            }
                        }
                    };

                    get_async_texture_loader()->enqueue_task(task);

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

void TextureManager::evict_if_needed() noexcept {
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
    get_logger()->info("[TextureManager] Evicting texture: " + oldest_it->first);
    m_texture_map.erase(oldest_it);
}
