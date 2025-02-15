#pragma once

#include <assimp/scene.h>
#include <scene/model/model.h>
#include <scene/resource/resource_loader.h>

class ModelLoader : public ResourceLoader {
public:
    ModelLoader() = default;

    ~ModelLoader() override = default;

    std::shared_ptr<Resource> load(const std::filesystem::path &path) override;

    bool save(std::shared_ptr<Resource> resource, const std::filesystem::path &path) override;

private:
    [[nodiscard]] static std::shared_ptr<Model> load_from_assimp(const std::filesystem::path &path);

    static void process_node(const aiNode *node, const aiScene *scene, std::vector<Mesh> &mesh_list,
                             const std::filesystem::path &path);

    static Mesh process_mesh(aiMesh *mesh, const aiScene *scene, const std::filesystem::path &path);

    static std::vector<std::shared_ptr<Texture>> load_textures(const aiMaterial *material,
                                                               const std::filesystem::path &path);
};

std::shared_ptr<ModelLoader> get_model_loader();
