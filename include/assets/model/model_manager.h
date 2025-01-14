#pragma once

#include <assets/model/model.h>
#include <assets/texture/texture_manager.h>
#include <assimp/scene.h>
#include <memory>
#include <mutex>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>

///                                                  Assimp
///                   Scene
///                  mRootNode------------------------------------------------------------
///       --------->mMeshes[]储存了真正的Mesh对象--                                         |
///       |         mMaterials[]               |                                         |
///       |               |                    |                                     Root Node
///       |               |                  Mesh                          ---------mChildren[]---------
///       |               |                mVertices[]                     |        mMeshes[]          |
///       |             Material           mNormals[]                      |                           |
///       |         GetTexture(type)       mTextureCoords[]             Child node       ---------->Child node
///       |                                mFaces[]--------------      mChildren[]-------|         mChildren[]
///       |                                mMaterialIndex       |      mMeshes[]场景中网格数组的索引   mMeshes[]
///       |                                                     |          |
///       |                                                    Face        |
///       |                                                  mIndices[]    |
///       |                                                                |
///       |                                                                |
///       ------------------------------------------------------------------
class ModelManager {
public:
    explicit ModelManager(const std::shared_ptr<TextureManager> &texture_manager);

    ~ModelManager() = default;

    bool find_model(const std::string &path);

    void add_model(const std::string &path);

    void remove_model(const std::string &path);

    std::shared_ptr<Model> get_model(const std::string &path);

private:
    std::unordered_map<std::string, std::shared_ptr<Model>> m_model_map;
    std::shared_ptr<TextureManager> m_texture_manager;
    std::mutex m_model_mutex;
    // We'll store the directory of the current model so we can use it in load_textures()
    std::string m_current_directory;
    std::shared_ptr<spdlog::logger> m_logger;

    // Load model from file using Assimp
    void load_model(const std::string &path, std::vector<Mesh> &mesh_list);

    // Process each node recursively
    void process_node(const aiNode *node, const aiScene *scene, std::vector<Mesh> &mesh_list) const;

    // Convert aiMesh to our Mesh
    Mesh process_mesh(aiMesh *mesh, const aiScene *scene) const;

    // Load textures using TextureManager
    std::vector<std::shared_ptr<Texture>> load_textures(const aiMaterial *material, aiTextureType ai_type,
                                                        TextureType texture_type) const;

    // A helper to get directory from path
    static std::string get_directory_from_path(const std::string &path);
};