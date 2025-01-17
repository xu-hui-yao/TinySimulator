#include <assets/shader/async_shader_loader.h>
#include <assets/shader/shader_loader.h>
#include <core/global.h>
#include <assets/fwd.h>

AsyncShaderLoader::AsyncShaderLoader() noexcept
    : m_stop_flag(false) {
}

AsyncShaderLoader::~AsyncShaderLoader() noexcept {
    {
        std::lock_guard lock(m_queue_mutex);
        m_stop_flag = true;
    }
    m_cv.notify_all();

    for (auto &th : m_workers) {
        if (th.joinable()) {
            th.join();
        }
    }
    m_workers.clear();
}

void AsyncShaderLoader::start(size_t num_threads) noexcept {
    m_stop_flag = false;
    for (size_t i = 0; i < num_threads; ++i) {
        m_workers.emplace_back(&AsyncShaderLoader::worker_thread, this);
    }
}

void AsyncShaderLoader::stop() noexcept {
    {
        std::lock_guard lock(m_queue_mutex);
        m_stop_flag = true;
    }
    m_cv.notify_all();

    for (auto &th : m_workers) {
        if (th.joinable()) {
            th.join();
        }
    }
    m_workers.clear();
}

void AsyncShaderLoader::enqueue_task(const ResourceTask &task) noexcept {
    {
        std::lock_guard lock(m_queue_mutex);
        m_task_queue.push(task);
    }
    m_cv.notify_one();
}

void AsyncShaderLoader::worker_thread() noexcept {
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

        if (task.task_type == ResourceTaskType::Load || task.task_type == ResourceTaskType::Reload) {
            auto res = load_resource_impl(task);
            if (task.on_loaded) {
                task.on_loaded(res);
            }
        } else if (task.task_type == ResourceTaskType::Delete) {
            if (task.on_deleted) {
                task.on_deleted();
            }
        }
    }
}

std::shared_ptr<Resource> AsyncShaderLoader::load_resource_impl(const ResourceTask &task) noexcept {
    // reuse the synchronous loader
    auto shader_loader = get_shader_loader();
    auto resource = shader_loader->load(task.file_path);
    if (!resource) {
        get_logger()->error("[AsyncShaderLoader] Failed to load: " + task.file_path.str());
    }
    return resource;
}
