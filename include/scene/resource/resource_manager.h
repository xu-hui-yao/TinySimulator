#pragma once

#include <any>
#include <scene/resource/resource.h>

/**
 * @brief Base interface for resource manager.
 *        It typically stores loaded resources, handles hot reloading, etc.
 */
class ResourceManager {
public:
    virtual ~ResourceManager() = default;

    /**
     * @brief Synchronously load a resource from file and store it internally.
     * @param path The file path.
     * @param param The descriptor of internal resource
     * @return The resource pointer if successful, or throws/returns nullptr on failure.
     */
    virtual std::shared_ptr<Resource> load_resource(const std::filesystem::path &path,
                                                    const std::unordered_map<std::string, std::any> &param) = 0;

    /**
     * @brief Asynchronously load a resource from file.
     * @param path The file path.
     * @param param The descriptor of internal resource
     */
    virtual void load_resource_async(const std::filesystem::path &path,
                                     const std::unordered_map<std::string, std::any> &param) = 0;

    /**
     * @brief Synchronously remove a resource from internal storage.
     * @param path The file path (or key).
     */
    virtual void remove_resource(const std::filesystem::path &path) = 0;

    /**
     * @brief Asynchronously remove a resource from internal storage.
     * @param path The file path (or key).
     */
    virtual void remove_resource_async(const std::filesystem::path &path) = 0;

    /**
     * @brief Check if the resource is stored (exists in the manager).
     */
    virtual bool exist_resource(const std::filesystem::path &path) = 0;

    /**
     * @brief Get a resource from internal storage by path.
     * @param path The file path (or key).
     * @return The resource pointer if exists, otherwise throw or return nullptr.
     */
    virtual std::shared_ptr<Resource> get_resource(const std::filesystem::path &path) = 0;

    /**
     * @brief Enable or disable hot-reload checking.
     * @param enable True to enable, false to disable.
     * @param interval The checking interval in seconds.
     */
    virtual void enable_hot_reload(bool enable, std::chrono::seconds interval) = 0;
};
