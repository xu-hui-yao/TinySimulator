#include <assets/model/model_loader.h>
#include <assets/fwd.h>
#include <assets/texture/texture_manager.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <spdlog/spdlog.h>
#include <core/global.h>

std::shared_ptr<Resource> ModelLoader::load(const filesystem::path &path) {
    auto model = load_from_assimp(path);
    if (!model) {
        get_logger()->error("[ModelLoader] Failed to load model from '{}'" + path.make_absolute().str());
    }
    return model;
}

bool ModelLoader::save(std::shared_ptr<Resource> resource, const filesystem::path &path) {
    get_logger()->warn("[ModelLoader] Saving models is not implemented");
    return false;
}

std::shared_ptr<Model> ModelLoader::load_from_assimp(const filesystem::path &path) {
    std::string filepath = path.make_absolute().str();
    Assimp::Importer importer;

    const aiScene *scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs |
                                                           aiProcess_GenNormals | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        get_logger()->error("[ModelLoader] Assimp error: {}", importer.GetErrorString());
        return nullptr;
    }

    std::vector<Mesh> all_meshes;

    process_node(scene->mRootNode, scene, all_meshes, path.parent_path());

    auto model = std::make_shared<Model>(path, std::move(all_meshes));
    return model;
}

void ModelLoader::process_node(const aiNode *node, const aiScene *scene, std::vector<Mesh> &mesh_list,
                               const filesystem::path &path) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *ai_mesh = scene->mMeshes[node->mMeshes[i]];
        Mesh mesh       = process_mesh(ai_mesh, scene, path);
        mesh_list.push_back(std::move(mesh));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene, mesh_list, path);
    }
}

Mesh ModelLoader::process_mesh(aiMesh *mesh, const aiScene *scene, const filesystem::path &path) {
    std::vector<Vertex> vertices;
    vertices.reserve(mesh->mNumVertices);

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vert{};

        // position
        vert.position.x = mesh->mVertices[i].x;
        vert.position.y = mesh->mVertices[i].y;
        vert.position.z = mesh->mVertices[i].z;

        // normal
        if (mesh->HasNormals()) {
            vert.normal.x = mesh->mNormals[i].x;
            vert.normal.y = mesh->mNormals[i].y;
            vert.normal.z = mesh->mNormals[i].z;
        }

        // texture coords
        if (mesh->mTextureCoords[0]) {
            vert.texture_coords.x = mesh->mTextureCoords[0][i].x;
            vert.texture_coords.y = mesh->mTextureCoords[0][i].y;
        }

        // tangent/bi_tangent
        if (mesh->mTangents) {
            vert.tangent.x = mesh->mTangents[i].x;
            vert.tangent.y = mesh->mTangents[i].y;
            vert.tangent.z = mesh->mTangents[i].z;
        }
        if (mesh->mBitangents) {
            vert.bi_tangent.x = mesh->mBitangents[i].x;
            vert.bi_tangent.y = mesh->mBitangents[i].y;
            vert.bi_tangent.z = mesh->mBitangents[i].z;
        }

        vertices.push_back(vert);
    }

    std::vector<GLuint> indices;
    indices.reserve(mesh->mNumFaces * 3);
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        const aiFace &face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    aiMaterial *material                           = scene->mMaterials[mesh->mMaterialIndex];
    std::vector<std::shared_ptr<Texture>> textures = load_textures(material, path);

    return { std::move(vertices), std::move(indices), std::move(textures) };
}

std::vector<std::shared_ptr<Texture>> ModelLoader::load_textures(const aiMaterial *material,
                                                                 const filesystem::path &path) {
    std::vector<std::shared_ptr<Texture>> results;

    for (int i = 0; i < static_cast<int>(aiTextureType_MAYA_SPECULAR_ROUGHNESS); i++) {
        auto ai_type       = static_cast<aiTextureType>(i);
        unsigned int count = material->GetTextureCount(ai_type);
        for (unsigned int j = 0; j < count; j++) {
            aiString str;
            material->GetTexture(ai_type, j, &str);
            std::string full_tex_path = path.make_absolute().str() + "/" + std::string(str.C_Str());

            if (!get_texture_manager()->exist_resource(filesystem::path(full_tex_path))) {
                get_texture_manager()->load_resource(filesystem::path(full_tex_path));
            }
            auto texture_ptr =
                std::dynamic_pointer_cast<Texture>(get_texture_manager()->get_resource(filesystem::path(full_tex_path)));
            texture_ptr->set_type(static_cast<TextureType>(i));

            results.push_back(texture_ptr);
        }
    }

    return results;
}
