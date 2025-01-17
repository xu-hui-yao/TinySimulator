#pragma once

#include <assets/resource.h>
#include <functional>

/**
 * @brief Task type for asynchronous operations.
 */
enum class ResourceTaskType { Load, Reload, Delete };

/**
 * @brief A resource task to be processed asynchronously.
 */
struct ResourceTask {
    ResourceTaskType task_type;
    filesystem::path file_path;
    // The callback when loading/reloading is done.
    std::function<void(std::shared_ptr<Resource>)> on_loaded = nullptr;
    // The callback when deleting is done.
    std::function<void()> on_deleted = nullptr;
    // Old resource for reload tasks (optional).
    std::shared_ptr<Resource> old_resource = nullptr;
};

/**
 * @brief Interface for asynchronous resource loading/deleting.
 */
class AsyncResourceLoader {
public:
    virtual ~AsyncResourceLoader() = default;

    /**
     * @brief Start the background worker threads.
     * @param num_threads The number of threads to spawn.
     */
    virtual void start(size_t num_threads) = 0;

    /**
     * @brief Stop and join all background worker threads.
     */
    virtual void stop() = 0;

    /**
     * @brief Enqueue a resource task for loading/reloading/deleting.
     */
    virtual void enqueue_task(const ResourceTask &task) = 0;
};
