#pragma once

#include <assets/model/model.h>
#include <assets/resource/resource_loader.h>
#include <assimp/scene.h>

class ModelLoader : public ResourceLoader {
public:
    ModelLoader() = default;

    ~ModelLoader() override = default;

    std::shared_ptr<Resource> load(const filesystem::path &path) override;

    bool save(std::shared_ptr<Resource> resource, const filesystem::path &path) override;

private:
    [[nodiscard]] static std::shared_ptr<Model> load_from_assimp(const filesystem::path &path) ;

    static void process_node(const aiNode *node, const aiScene *scene, std::vector<Mesh> &mesh_list,
                      const filesystem::path &path);

    static Mesh process_mesh(aiMesh *mesh, const aiScene *scene, const filesystem::path &path) ;

    static std::vector<std::shared_ptr<Texture>> load_textures(const aiMaterial *material, const filesystem::path &path) ;
};
