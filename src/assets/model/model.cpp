#include <assets/model/model.h>
#include <stdexcept>

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<GLuint> &indices,
           const std::vector<std::shared_ptr<Texture>> &textures)
    : vertices(vertices), indices(indices), textures(textures), vbo(0), ebo(0), vao(0) {}

Mesh::~Mesh() {
    if (vbo)
        glDeleteBuffers(1, &vbo);
    if (ebo)
        glDeleteBuffers(1, &ebo);
    if (vao)
        glDeleteVertexArrays(1, &vao);
}

Mesh::Mesh(Mesh &&other) noexcept
    : vertices(std::move(other.vertices)), indices(std::move(other.indices)), textures(std::move(other.textures)),
      vbo(other.vbo), ebo(other.ebo), vao(other.vao) {
    other.vbo = 0;
    other.ebo = 0;
    other.vao = 0;
}

Mesh &Mesh::operator=(Mesh &&other) noexcept {
    if (this != &other) {
        if (vbo)
            glDeleteBuffers(1, &vbo);
        if (ebo)
            glDeleteBuffers(1, &ebo);
        if (vao)
            glDeleteVertexArrays(1, &vao);

        vertices = std::move(other.vertices);
        indices  = std::move(other.indices);
        textures = std::move(other.textures);
        vbo      = other.vbo;
        ebo      = other.ebo;
        vao      = other.vao;

        other.vbo = 0;
        other.ebo = 0;
        other.vao = 0;
    }
    return *this;
}

void Mesh::upload_to_gpu() {
    if (vao != 0 || ebo != 0 || vbo != 0) {
        return;
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    if (!vao || !vbo || !ebo) {
        throw std::runtime_error("Failed to generate OpenGL buffers");
    }

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)), vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(GLuint)), indices.data(),
                 GL_STATIC_DRAW);

    // Vertex attribute: position (layout = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, position)));

    // Vertex attribute: normal (layout = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, normal)));

    // Vertex attribute: texture coordinates (layout = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, texture_coords)));

    // Vertex attribute: tangent (layout = 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, tangent)));

    // Vertex attribute: bi-tangent (layout = 4)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, bi_tangent)));

    // Vertex attribute: bone IDs (layout = 5)
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, M_MAX_BONE_INFLUENCE, GL_INT, sizeof(Vertex),
                           reinterpret_cast<void *>(offsetof(Vertex, bone_ids)));

    // Vertex attribute: bone weights (layout = 6)
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, M_MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, weights)));

    glBindVertexArray(0);

    for (const auto &texture : textures) {
        texture->upload();
    }
}

Model::Model(const std::vector<Mesh> &meshes) : meshes(meshes) {}

Model::~Model() = default;

Model::Model(Model &&other) noexcept : meshes(std::move(other.meshes)) {}

Model &Model::operator=(Model &&other) noexcept {
    if (this != &other) {
        meshes = std::move(other.meshes);
    }
    return *this;
}

void Model::upload_to_gpu() {
    for (auto &mesh : meshes) {
        mesh.upload_to_gpu();
    }
}
