#pragma once

#include <condition_variable>
#include <queue>
#include <scene/resource/async_resource_loader.h>
#include <scene/texture/texture_loader.h>
#include <thread>
#include <vector>

/**
 * @brief An asynchronous loader specialized for Texture-based resources (or any Resource).
 *        Internally uses a queue and a thread pool.
 */
class AsyncTextureLoader : public AsyncResourceLoader {
public:
    explicit AsyncTextureLoader() noexcept;

    ~AsyncTextureLoader() noexcept override;

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

std::shared_ptr<AsyncTextureLoader> get_async_texture_loader();
