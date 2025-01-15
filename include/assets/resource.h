#pragma once

#include <core/filesystem/path.h>
#include <utility>

struct ResourceDescriptor {
    virtual ~ResourceDescriptor() = default;
};

/**
 * @brief The base interface for any Resource in the engine.
 *        It only handles the essential load/unload and path retrieval.
 */
class Resource {
public:
    explicit Resource(filesystem::path path) : m_path(std::move(path)) {}

    virtual ~Resource() = default;

    /**
     * @brief Upload resource data from CPU/GPU.
     */
    virtual void upload(std::shared_ptr<ResourceDescriptor> resource_descriptor) = 0;

    /**
     * @brief Unload resource data from CPU/GPU.
     */
    virtual void unload() = 0;

    /**
     * @brief Return the resource file path (if any).
     */
    [[nodiscard]] filesystem::path get_path() const { return m_path; }

protected:
    filesystem::path m_path;
};
