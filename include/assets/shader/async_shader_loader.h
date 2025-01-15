#pragma once

#include <assets/async_resource_loader.h>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class AsyncShaderLoader : public AsyncResourceLoader {
public:
    AsyncShaderLoader() noexcept;

    ~AsyncShaderLoader() noexcept override;

    void start(size_t num_threads) noexcept override;

    void stop() noexcept override;

    void enqueue_task(const ResourceTask &task) noexcept override;

private:
    std::vector<std::thread> m_workers;
    std::queue<ResourceTask> m_task_queue;
    std::mutex m_queue_mutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_stop_flag;

    void worker_thread() noexcept;

    static std::shared_ptr<Resource> load_resource_impl(const ResourceTask &task) noexcept;
};
