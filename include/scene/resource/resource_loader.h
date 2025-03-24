#pragma once

#include <any>
#include <unordered_map>
#include <filesystem>
#include <scene/resource/resource.h>

/**
 * @brief The synchronous (blocking) loader interface for a certain Resource type.
 */
class ResourceLoader {
public:
    virtual ~ResourceLoader() = default;

    /**
     * @brief Load resource from file synchronously, returning a Resource pointer.
     * @param path The file path.
     * @param param The descriptor of internal resource
     * @return std::shared_ptr<Resource> if load succeeds, or nullptr on failure.
     */
    virtual std::shared_ptr<Resource> load(const std::filesystem::path &path,
                                           const std::unordered_map<std::string, std::any> &param) = 0;

    /**
     * @brief Load resource from file synchronously, returning a Resource pointer.
     * @param resource resource pointer.
     * @param path The file path.
     * @return std::shared_ptr<Resource> if load succeeds, or nullptr on failure.
     */
    virtual bool save(std::shared_ptr<Resource> resource, const std::filesystem::path &path) = 0;
};
