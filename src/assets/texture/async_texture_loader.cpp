#include <assets/texture/async_texture_loader.h>
#include <iostream>

AsyncTextureLoader::AsyncTextureLoader() noexcept : m_stop_flag(false) {}

AsyncTextureLoader::~AsyncTextureLoader() noexcept {
    {
        std::lock_guard lock(m_queue_mutex);
        m_stop_flag = true;
    }
    m_cv.notify_all();

    for (auto &thread : m_workers) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    m_workers.clear();
}

void AsyncTextureLoader::start(size_t num_threads) noexcept {
    m_stop_flag = false;
    for (size_t i = 0; i < num_threads; ++i) {
        m_workers.emplace_back(&AsyncTextureLoader::worker_thread, this);
    }
}

void AsyncTextureLoader::stop() noexcept {
    {
        std::lock_guard lock(m_queue_mutex);
        m_stop_flag = true;
    }
    m_cv.notify_all();

    for (auto &thread : m_workers) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    m_workers.clear();
}

void AsyncTextureLoader::enqueue_task(const ResourceTask &task) noexcept {
    {
        std::lock_guard lock(m_queue_mutex);
        m_task_queue.push(task);
    }
    m_cv.notify_one();
}

void AsyncTextureLoader::worker_thread() noexcept {
    while (true) {
        ResourceTask task;
        {
            std::unique_lock lock(m_queue_mutex);
            m_cv.wait(lock, [this] { return !m_task_queue.empty() || m_stop_flag.load(); });
            if (m_stop_flag && m_task_queue.empty()) {
                return;
            }
            task = m_task_queue.front();
            m_task_queue.pop();
        }

        // Process
        if (task.task_type == ResourceTaskType::Load || task.task_type == ResourceTaskType::Reload) {
            auto res = load_resource_impl(task);
            if (task.on_loaded) {
                task.on_loaded(res);
            }
        } else if (task.task_type == ResourceTaskType::Delete) {
            // if there's specific cleanup logic, do it here
            if (task.on_deleted) {
                task.on_deleted();
            }
        }
    }
}

std::shared_ptr<Resource> AsyncTextureLoader::load_resource_impl(const ResourceTask &task) noexcept {
    // We can rely on the sync loader:
    auto resource = m_sync_loader.load(task.file_path);
    if (!resource) {
        std::cerr << "[AsyncTextureLoader] Failed to load: " << task.file_path << std::endl;
        return nullptr;
    }
    return resource;
}
