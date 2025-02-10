#pragma once

#include <assets/resource/resource.h>
#include <filesystem>
#include <core/filesystem/path.h>

/**
 * @brief The synchronous (blocking) loader interface for a certain Resource type.
 */
class ResourceLoader {
public:
    virtual ~ResourceLoader() = default;

    /**
     * @brief Load resource from file synchronously, returning a Resource pointer.
     * @param path The file path.
     * @return std::shared_ptr<Resource> if load succeeds, or nullptr on failure.
     */
    virtual std::shared_ptr<Resource> load(const filesystem::path &path) = 0;

    /**
     * @brief Load resource from file synchronously, returning a Resource pointer.
     * @param resource resource pointer.
     * @param path The file path.
     * @return std::shared_ptr<Resource> if load succeeds, or nullptr on failure.
     */
    virtual bool save(std::shared_ptr<Resource> resource, const filesystem::path &path) = 0;
};
