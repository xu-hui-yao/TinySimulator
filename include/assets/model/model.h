#pragma once

#include <assets/texture/texture.h>
#include <core/common.h>
#include <glad.h>
#include <glm/glm.hpp>
#include <memory>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texture_coords;
    glm::vec3 tangent;
    glm::vec3 bi_tangent;
    GLint bone_ids[M_MAX_BONE_INFLUENCE];
    GLfloat weights[M_MAX_BONE_INFLUENCE];
};

class Mesh {
public:
    Mesh(const std::vector<Vertex> &vertices, const std::vector<GLuint> &indices,
         const std::vector<std::shared_ptr<Texture>> &textures);

    ~Mesh();

    Mesh(Mesh &&other) noexcept;

    Mesh &operator=(Mesh &&other) noexcept;

    void upload_to_gpu();

private:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<std::shared_ptr<Texture>> textures;
    GLuint vbo, ebo, vao;
};

class Model {
public:
    explicit Model(const std::vector<Mesh> &meshes);

    ~Model();

    Model(Model &&other) noexcept;

    Model &operator=(Model &&other) noexcept;

    void upload_to_gpu();

private:
    std::vector<Mesh> meshes;
};
