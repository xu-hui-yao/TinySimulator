#include <assets/model/model_manager.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <stdexcept>

ModelManager::ModelManager(const std::shared_ptr<TextureManager> &texture_manager)
    : m_texture_manager(texture_manager) {}

bool ModelManager::find_model(const std::string &path) {
    std::lock_guard lock(m_model_mutex);
    return m_model_map.find(path) != m_model_map.end();
}

void ModelManager::add_model(const std::string &path) {
    std::lock_guard lock(m_model_mutex);
    if (m_model_map.find(path) != m_model_map.end()) {
        m_logger->info("Model already exists: ", path);
        return;
    }
    std::vector<Mesh> mesh_list;
    load_model(path, mesh_list);

    // After loading all meshes, create the Model
    auto model_ptr    = std::make_shared<Model>(mesh_list);
    m_model_map[path] = model_ptr;
}

void ModelManager::remove_model(const std::string &path) {
    std::lock_guard lock(m_model_mutex);
    auto it = m_model_map.find(path);
    if (it != m_model_map.end()) {
        m_model_map.erase(it);
    } else {
        m_logger->info("Model does not exist: ", path);
    }
}

std::shared_ptr<Model> ModelManager::get_model(const std::string &path) {
    auto it = m_model_map.find(path);
    if (it == m_model_map.end()) {
        m_logger->error("Model does not exist: ", path);
        throw std::runtime_error("Model does not exist: " + path);
    }
    return it->second;
}

void ModelManager::load_model(const std::string &path, std::vector<Mesh> &mesh_list) {
    // Extract directory from path
    m_current_directory = get_directory_from_path(path);

    Assimp::Importer importer;
    // aiProcess_Triangulate: Convert polygons to triangles
    // aiProcess_FlipUVs: Flip texture coordinates on Y axis
    // aiProcess_GenNormals: Generate normals if not present
    // aiProcess_CalcTangentSpace: Generate tangent and bi-tangent
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals |
                                                       aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error("Assimp error: " + std::string(importer.GetErrorString()));
    }

    // Recursively process the root node
    process_node(scene->mRootNode, scene, mesh_list);
}

void ModelManager::process_node(const aiNode *node, const aiScene *scene, std::vector<Mesh> &mesh_list) const {
    // For each mesh index in this node, convert to our Mesh and store
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *ai_mesh = scene->mMeshes[node->mMeshes[i]];
        mesh_list.push_back(process_mesh(ai_mesh, scene));
    }
    // Recursively process children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene, mesh_list);
    }
}

Mesh ModelManager::process_mesh(aiMesh *mesh, const aiScene *scene) const {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<std::shared_ptr<Texture>> textures;

    // Fill vertices
    vertices.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex{};

        // Position
        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        // Normal
        if (mesh->HasNormals()) {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        } else {
            vertex.normal = glm::vec3(0.0f);
        }

        // Texture coordinates
        if (mesh->mTextureCoords[0]) {
            vertex.texture_coords.x = mesh->mTextureCoords[0][i].x;
            vertex.texture_coords.y = mesh->mTextureCoords[0][i].y;
        } else {
            vertex.texture_coords = glm::vec2(0.0f);
        }

        // Tangent
        if (mesh->mTangents) {
            vertex.tangent.x = mesh->mTangents[i].x;
            vertex.tangent.y = mesh->mTangents[i].y;
            vertex.tangent.z = mesh->mTangents[i].z;
        } else {
            vertex.tangent = glm::vec3(0.0f);
        }

        // Bi-tangent
        if (mesh->mBitangents) {
            vertex.bi_tangent.x = mesh->mBitangents[i].x;
            vertex.bi_tangent.y = mesh->mBitangents[i].y;
            vertex.bi_tangent.z = mesh->mBitangents[i].z;
        } else {
            vertex.bi_tangent = glm::vec3(0.0f);
        }

        // Bone IDs / Weights not handled in this example

        vertices.push_back(vertex);
    }

    // Fill indices
    indices.reserve(mesh->mNumFaces * 3); // each face is a triangle
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Get material and load textures
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        // Diffuse
        auto diffuse_maps = load_textures(material, aiTextureType_DIFFUSE, EDiffuse);
        textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

        // Specular
        auto specular_maps = load_textures(material, aiTextureType_SPECULAR, ESpecular);
        textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());

        // Normal (aiTextureType_HEIGHT is often used for normal)
        auto normal_maps = load_textures(material, aiTextureType_HEIGHT, ENormal);
        textures.insert(textures.end(), normal_maps.begin(), normal_maps.end());

        // Height (ambient sometimes used as height map)
        auto height_maps = load_textures(material, aiTextureType_AMBIENT, EHeight);
        textures.insert(textures.end(), height_maps.begin(), height_maps.end());
    }

    return { vertices, indices, textures };
}

std::vector<std::shared_ptr<Texture>> ModelManager::load_textures(const aiMaterial *material, aiTextureType ai_type,
                                                                  TextureType texture_type) const {
    std::vector<std::shared_ptr<Texture>> result;
    unsigned int count = material->GetTextureCount(ai_type);
    for (unsigned int i = 0; i < count; i++) {
        aiString str;
        material->GetTexture(ai_type, i, &str);
        // Combine directory and file name
        std::string texture_path = m_current_directory + "/" + std::string(str.C_Str());

        // Check in texture_manager_ first
        if (!m_texture_manager->exist_texture(texture_path)) {
            // If not found, add it
            m_texture_manager->add_texture(texture_path, texture_type);
        }
        // Then get the shared_ptr
        auto tex_ptr = m_texture_manager->get_texture(texture_path);
        result.push_back(tex_ptr);
    }
    return result;
}

std::string ModelManager::get_directory_from_path(const std::string &path) {
    // Simple approach: find last '/' or '\'
    // (In real project, use a more robust approach or cross-platform library)
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) {
        return "."; // no directory found
    }
    return path.substr(0, pos);
}