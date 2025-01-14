#pragma once

#include <assets/async_resource_loader.h>
#include <assets/texture/texture_loader.h>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

/**
 * @brief An asynchronous loader specialized for Texture-based resources (or any Resource).
 *        Internally uses a queue and a thread pool.
 */
class AsyncTextureLoader : public AsyncResourceLoader {
public:
    AsyncTextureLoader() noexcept;

    ~AsyncTextureLoader() noexcept override;

    void start(size_t num_threads) noexcept override;

    void stop() noexcept override;

    void enqueue_task(const ResourceTask &task) noexcept override;

private:
    TextureLoader m_sync_loader; // We can reuse the sync loader inside
    std::vector<std::thread> m_workers;
    std::queue<ResourceTask> m_task_queue;
    std::mutex m_queue_mutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_stop_flag;

    void worker_thread() noexcept;

    std::shared_ptr<Resource> load_resource_impl(const ResourceTask &task) noexcept;
};
