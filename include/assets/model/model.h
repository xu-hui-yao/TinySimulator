#pragma once

#include <assets/resource.h>
#include <assets/texture/texture.h>
#include <glm/glm.hpp>
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

    void upload_to_gpu() noexcept;

    void unload_from_gpu() noexcept;

    [[nodiscard]] const std::vector<std::shared_ptr<Texture>> &get_textures() const noexcept;

private:
    std::vector<Vertex> m_vertices;
    std::vector<GLuint> m_indices;
    std::vector<std::shared_ptr<Texture>> m_textures;

    GLuint m_vao{ 0 }, m_vbo{ 0 }, m_ebo{ 0 };
};

class Model : public Resource {
public:
    explicit Model(filesystem::path path, std::vector<Mesh> meshes) noexcept;

    ~Model() noexcept override;

    Model(Model &&other) noexcept;

    Model &operator=(Model &&other) noexcept;

    void upload(std::shared_ptr<ResourceDescriptor> resource_descriptor) noexcept override;

    void unload() noexcept override;

    [[nodiscard]] const std::vector<Mesh> &get_meshes() const noexcept;

private:
    std::vector<Mesh> m_meshes;
};
