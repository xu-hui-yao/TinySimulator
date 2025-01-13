#include <assets/model/model.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>
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
        std::cout << "Model already generated and bound." << std::endl;
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
        texture->upload_to_gpu();
    }
}

Model::Model(const std::string &path) { load_model(path); }

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

void Model::load_model(std::string const &path) {
    Assimp::Importer importer;
    // aiProcess_Triangulate: If the model is not fully composed of triangles,
    // this flag converts all primitive shapes into triangles.
    // aiProcess_FlipUVs: Flips the texture coordinates along the y-axis during processing.
    // aiProcess_GenNormals: Generates normals for each vertex if the model does not already include them.
    // aiProcess_SplitLargeMeshes: Splits larger meshes into smaller sub-meshes, useful if your rendering has a maximum
    // vertex limit and can only handle smaller meshes.
    // aiProcess_OptimizeMeshes: Combines multiple small meshes into a larger one to reduce draw calls and optimize
    // rendering.
    // aiProcess_CalcTangentSpace: Automatically calculates tangents and bi-tangents for the mesh.
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals |
                                                       aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error("Assimp error " + std::string(importer.GetErrorString()));
    }
    process_node(scene->mRootNode, scene);
}

void Model::process_node(aiNode *node, const aiScene *scene) {
    for (int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(process_mesh(mesh, scene));
    }
    for (int i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene);
    }
}

Mesh Model::process_mesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;

    // 初始化每个vertex
    for (GLuint i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex{};
        glm::vec3 vector3;
        glm::vec2 vector2;
        // 顶点位置
        vector3.x = mesh->mVertices[i].x;
        vector3.y = mesh->mVertices[i].y;
        vector3.z = mesh->mVertices[i].z;
        vertex.position = vector3;
        // 顶点法向量
        if (mesh->HasNormals()) {
            vector3.x = mesh->mNormals[i].x;
            vector3.y = mesh->mNormals[i].y;
            vector3.z = mesh->mNormals[i].z;
            vertex.normal = vector3;
        } else {
            vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        // 材质
        if (mesh->mTextureCoords[0]) {
            // 材质
            vector2.x = mesh->mTextureCoords[0][i].x;
            vector2.y = mesh->mTextureCoords[0][i].y;
            vertex.texture_coords = vector2;
        } else {
            vertex.texture_coords = glm::vec2(0.0f, 0.0f);
        }
        // 切线
        if (mesh->mBitangents) {
            vector3.x = mesh->mTangents[i].x;
            vector3.y = mesh->mTangents[i].y;
            vector3.z = mesh->mTangents[i].z;
            vertex.tangent = vector3;
        } else {
            vertex.tangent = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        // 副切线
        if (mesh->mBitangents) {
            vector3.x = mesh->mBitangents[i].x;
            vector3.y = mesh->mBitangents[i].y;
            vector3.z = mesh->mBitangents[i].z;
            vertex.bi_tangent = vector3;
        } else {
            vertex.bi_tangent = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        vertices.push_back(vertex);
    }

    // 初始化每个indices
    for (GLuint i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (GLuint j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    // 1. diffuse maps
    std::vector<Texture> diffuseMaps = process_texture(material, aiTextureType_DIFFUSE,
                                                              TextureType::EDiffuseType);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. specular maps
    std::vector<Texture> specularMaps = process_texture(material, aiTextureType_SPECULAR,
                                                               TextureType::ESpecularType);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    std::vector<Texture> normalMaps = process_texture(material, aiTextureType_HEIGHT, TextureType::ENormalType);
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    std::vector<Texture> heightMaps = process_texture(material, aiTextureType_AMBIENT, TextureType::EHeightType);
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    Mesh mesh1 = Mesh(vertices, indices, textures);

    return mesh1;
}

std::vector<Texture> Model::process_texture(aiMaterial *material, aiTextureType type, TextureType texture_type) {
    static std::vector<Texture> loaded_texture;
    std::vector<Texture> textures;
    for (GLuint i = 0; i < material->GetTextureCount(type); i++) {  // 检查储存在材质中纹理的数量
        aiString str;
        material->GetTexture(type, i, &str);  // 获取每个纹理的文件位置，它会将结果储存在一个aiString中
        // 优化纹理重用
        GLboolean skip = false;
        for (auto &iter: loaded_texture) {
            if (std::strcmp(iter.path.data(), str.C_Str()) == 0) {
                textures.push_back(iter);
                skip = true;
                break;
            }
        }
        if (!skip) {
            auto texture = Texture::texture2DFromFile(directory + '/' + std::string(const_cast<GLchar *>(str.C_Str())),
                                                      textureType, false);
            textures.push_back(*texture);
            loaded_texture.push_back(*texture);
        }
    }
    return textures;
}
