#include <assets/model/async_model_loader.h>
#include <assets/fwd.h>
#include <core/global.h>

AsyncModelLoader::AsyncModelLoader() noexcept : m_stop_flag(false) {}

AsyncModelLoader::~AsyncModelLoader() noexcept {
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

void AsyncModelLoader::start(size_t num_threads) noexcept {
    m_stop_flag = false;
    for (size_t i = 0; i < num_threads; ++i) {
        m_workers.emplace_back(&AsyncModelLoader::worker_thread, this);
    }
}

void AsyncModelLoader::stop() noexcept {
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

void AsyncModelLoader::enqueue_task(const ResourceTask &task) noexcept {
    {
        std::lock_guard lock(m_queue_mutex);
        m_task_queue.push(task);
    }
    m_cv.notify_one();
}

void AsyncModelLoader::worker_thread() noexcept {
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

std::shared_ptr<Resource> AsyncModelLoader::load_resource_impl(const ResourceTask &task) noexcept {
    auto resource = get_model_loader()->load(task.file_path);
    if (!resource) {
        get_logger()->error("[AsyncModelLoader] Failed to load: " + task.file_path.str());
    }
    return resource;
}
