#pragma once

#include <assimp/scene.h>
#include <scene/model/model.h>
#include <scene/resource/resource_loader.h>
#include <any>

class ModelLoader : public ResourceLoader {
public:
    ModelLoader() = default;

    ~ModelLoader() override = default;

    std::shared_ptr<Resource> load(const std::filesystem::path &path,
                                   const std::unordered_map<std::string, std::any> &param) override;

    bool save(std::shared_ptr<Resource> resource, const std::filesystem::path &path) override;

    static constexpr std::string internal_prefix = "internal://primitive/";
    static constexpr int internal_prefix_length  = internal_prefix.length();

private:
    [[nodiscard]] static std::shared_ptr<Model> load_from_assimp(const std::filesystem::path &path);

    static void process_node(const aiNode *node, const aiScene *scene, std::vector<std::shared_ptr<Mesh>> &mesh_list,
                             const std::filesystem::path &path);

    static Mesh process_mesh(aiMesh *mesh, const aiScene *scene, const std::filesystem::path &path);

    static std::vector<std::shared_ptr<Texture>> load_textures(const aiMaterial *material,
                                                               const std::filesystem::path &path);
};

std::shared_ptr<ModelLoader> get_model_loader();
