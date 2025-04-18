#if defined(_WIN32)
#undef APIENTRY
#endif
#include <glad.h>
#include <scene/model/model.h>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices,
           std::vector<std::shared_ptr<Texture>> textures) noexcept
    : m_vertices(std::move(vertices)), m_indices(std::move(indices)), m_textures(std::move(textures)) {}

Mesh::~Mesh() noexcept { unload_from_gpu(); }

Mesh::Mesh(Mesh &&other) noexcept
    : m_vertices(std::move(other.m_vertices)), m_indices(std::move(other.m_indices)),
      m_textures(std::move(other.m_textures)), m_vao(other.m_vao), m_vbo(other.m_vbo), m_ebo(other.m_ebo) {
    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_ebo = 0;
}

Mesh &Mesh::operator=(Mesh &&other) noexcept {
    if (this != &other) {
        unload_from_gpu();

        m_vertices = std::move(other.m_vertices);
        m_indices  = std::move(other.m_indices);
        m_textures = std::move(other.m_textures);
        m_vao      = other.m_vao;
        m_vbo      = other.m_vbo;
        m_ebo      = other.m_ebo;

        other.m_vao = 0;
        other.m_vbo = 0;
        other.m_ebo = 0;
    }
    return *this;
}

void Mesh::upload_to_gpu(bool vertices_changed, bool indices_changed) noexcept {
    if (m_vao != 0 && m_vbo != 0 && m_ebo != 0) {
        return;
    }

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    if (!m_vao || !m_vbo || !m_ebo) {
        get_logger()->error("Failed to generate OpenGL buffers for Mesh");
        return;
    }

    glBindVertexArray(m_vao);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_vertices.size() * sizeof(Vertex)), m_vertices.data(),
                 vertices_changed ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_indices.size() * sizeof(GLuint)), m_indices.data(),
                 indices_changed ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

    // Position => layout = 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, position)));

    // Normal => layout = 1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, normal)));

    // Texture coords => layout = 2
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, texture_coords)));

    // Tangent => layout = 3
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, tangent)));

    // Bi_tangent => layout = 4
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, bi_tangent)));

    // Bone IDs => layout = 5
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, M_MAX_BONE_INFLUENCE, GL_INT, sizeof(Vertex),
                           reinterpret_cast<void *>(offsetof(Vertex, bone_ids)));

    // Weights => layout = 6
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, M_MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, weights)));

    glBindVertexArray(0);

    for (auto &texture : m_textures) {
        auto texture_descriptor = std::make_shared<TextureDescriptor>();
        if (texture) {
            texture->upload(texture_descriptor);
        }
    }
}

void Mesh::unload_from_gpu() noexcept {
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_ebo) {
        glDeleteBuffers(1, &m_ebo);
        m_ebo = 0;
    }
}

void Mesh::update_gpu_buffer() const noexcept {
    if (m_vbo == 0) {
        return;
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(m_vertices.size() * sizeof(Vertex)), m_vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

const std::vector<std::shared_ptr<Texture>> &Mesh::get_textures() const noexcept { return m_textures; }

GLuint Mesh::get_vao() const noexcept { return m_vao; }

GLuint Mesh::get_vbo() const noexcept { return m_vbo; }

GLuint Mesh::get_ebo() const noexcept { return m_ebo; }

size_t Mesh::get_indices_size() const noexcept { return m_indices.size(); }

std::vector<Vertex> &Mesh::get_vertices() noexcept { return m_vertices; }

const std::vector<Vertex> &Mesh::get_vertices() const noexcept { return m_vertices; }

const std::vector<GLuint> &Mesh::get_indices() const noexcept { return m_indices; }

//
// ================ Model ================
//
Model::Model(std::filesystem::path path, std::vector<std::shared_ptr<Mesh>> meshes) noexcept
    : Resource(std::move(path)), m_meshes(std::move(meshes)), vertices_changed(false), indices_changed(false) {}

Model::~Model() noexcept {
    for (auto &mesh : m_meshes) {
        mesh->unload_from_gpu();
    }
}

Model::Model(Model &&other) noexcept
    : Resource(other.m_path), m_meshes(std::move(other.m_meshes)), vertices_changed(other.vertices_changed),
      indices_changed(other.indices_changed) {}

Model &Model::operator=(Model &&other) noexcept {
    if (this != &other) {
        unload();
        m_path   = other.m_path;
        m_meshes = std::move(other.m_meshes);
    }
    return *this;
}

void Model::upload(std::shared_ptr<ResourceDescriptor> /**/) noexcept {
    for (auto &mesh : m_meshes) {
        mesh->upload_to_gpu(vertices_changed, indices_changed);
    }
}

void Model::unload() noexcept {
    for (auto &mesh : m_meshes) {
        mesh->unload_from_gpu();
    }
}

void Model::update_gpu_buffer() const {
    for (auto &mesh : m_meshes) {
        mesh->update_gpu_buffer();
    }
}

const std::vector<std::shared_ptr<Mesh>> &Model::get_meshes() const noexcept { return m_meshes; }

bool &Model::get_vertices_changed() noexcept { return vertices_changed; }

bool &Model::get_indices_changed() noexcept { return indices_changed; }
