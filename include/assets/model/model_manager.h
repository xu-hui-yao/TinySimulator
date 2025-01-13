#pragma once

#include <assets/model/model.h>
#include <assets/texture/texture_manager.h>
#include <memory>
#include <mutex>
#include <string>

class ModelManager {
public:
    explicit ModelManager(const std::shared_ptr<TextureManager> &texture_manager);

    ~ModelManager() = default;

    void add_model(const std::string &path);

    void remove_model(const std::string &path);

    std::shared_ptr<Model> get_model(const std::string &path);

private:
    std::unordered_map<std::string, std::shared_ptr<Model>> model_map;
    std::shared_ptr<TextureManager> texture_manager;
    std::mutex model_mutex;

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
    void load_model(std::string const &path);

    void process_node(aiNode *node, const aiScene *scene);

    Mesh process_mesh(aiMesh *mesh, const aiScene *scene);

    std::vector<Texture> process_texture(aiMaterial *material, aiTextureType type, TextureType texture_type);
};