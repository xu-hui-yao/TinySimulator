#pragma once

#include <core/fwd.h>
#include <glm/glm.hpp>
#include <scene/resource/resource.h>
#include <scene/texture/texture.h>
#include <vector>

struct Vertex {
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec2 texture_coords{};
    glm::vec3 tangent{};
    glm::vec3 bi_tangent{};
    GLint bone_ids[M_MAX_BONE_INFLUENCE]{};
    GLfloat weights[M_MAX_BONE_INFLUENCE]{};
};

class Mesh {
public:
    Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices,
         std::vector<std::shared_ptr<Texture>> textures) noexcept;

    ~Mesh() noexcept;

    Mesh(Mesh &&other) noexcept;

    Mesh &operator=(Mesh &&other) noexcept;

    Mesh(const Mesh &other) = delete;

    Mesh &operator=(const Mesh &other) = delete;

    void upload_to_gpu(bool vertices_changed, bool indices_changed) noexcept;

    void unload_from_gpu() noexcept;

    void update_gpu_buffer() const noexcept;

    [[nodiscard]] const std::vector<std::shared_ptr<Texture>> &get_textures() const noexcept;

    [[nodiscard]] GLuint get_vao() const noexcept;

    [[nodiscard]] GLuint get_vbo() const noexcept;

    [[nodiscard]] GLuint get_ebo() const noexcept;

    [[nodiscard]] size_t get_indices_size() const noexcept;

    [[nodiscard]] std::vector<Vertex> &get_vertices() noexcept;

    [[nodiscard]] const std::vector<Vertex> &get_vertices() const noexcept;

    [[nodiscard]] const std::vector<GLuint> &get_indices() const noexcept;

private:
    std::vector<Vertex> m_vertices;
    std::vector<GLuint> m_indices;
    std::vector<std::shared_ptr<Texture>> m_textures;

    GLuint m_vao{ 0 }, m_vbo{ 0 }, m_ebo{ 0 };
};

class Model : public Resource {
public:
    explicit Model(std::filesystem::path path, std::vector<std::shared_ptr<Mesh>> meshes) noexcept;

    ~Model() noexcept override;

    Model(Model &&other) noexcept;

    Model &operator=(Model &&other) noexcept;

    void upload(std::shared_ptr<ResourceDescriptor> resource_descriptor) noexcept override;

    void unload() noexcept override;

    void update_gpu_buffer() const;

    [[nodiscard]] const std::vector<std::shared_ptr<Mesh>> &get_meshes() const noexcept;

    bool& get_vertices_changed() noexcept;

    bool& get_indices_changed() noexcept;

private:
    std::vector<std::shared_ptr<Mesh>> m_meshes;
    bool vertices_changed;
    bool indices_changed;
};
