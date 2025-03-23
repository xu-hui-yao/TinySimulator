#include <scene/model/primitive_generator.h>
#include <scene/texture/texture.h>

std::shared_ptr<Model> PrimitiveGenerator::generate_cube(const std::unordered_map<std::string, std::any> &params) {
    const float width  = params.contains("width") ? std::any_cast<float>(params.at("width")) : 1.0f;
    const float height = params.contains("height") ? std::any_cast<float>(params.at("height")) : 1.0f;
    const float depth  = params.contains("depth") ? std::any_cast<float>(params.at("depth")) : 1.0f;

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<std::shared_ptr<Texture>> textures;

    try {
        if (params.contains("material")) {
            const auto &material = std::any_cast<std::unordered_map<std::string, std::any>>(params.at("material"));
            const auto color_r   = material.contains("color_r") ? std::any_cast<float>(material.at("color_r")) : 1.0f;
            const auto color_g   = material.contains("color_g") ? std::any_cast<float>(material.at("color_g")) : 1.0f;
            const auto color_b   = material.contains("color_b") ? std::any_cast<float>(material.at("color_b")) : 1.0f;
            const auto color_a   = material.contains("color_a") ? std::any_cast<float>(material.at("color_a")) : 1.0f;
            const auto type = material.contains("type") ? std::any_cast<TextureType>(material.at("type")) : EDiffuse;
            textures.push_back(Texture::create_solid_color(color_r, color_g, color_b, color_a, type));
        }
    } catch (const std::bad_any_cast &e) {
        get_logger()->error("Invalid material parameters: {}", e.what());
    }

    const glm::vec3 positions[] = { { -width / 2, -height / 2, depth / 2 },  { width / 2, -height / 2, depth / 2 },
                                    { width / 2, height / 2, depth / 2 },    { -width / 2, height / 2, depth / 2 },
                                    { width / 2, -height / 2, -depth / 2 },  { -width / 2, -height / 2, -depth / 2 },
                                    { -width / 2, height / 2, -depth / 2 },  { width / 2, height / 2, -depth / 2 },
                                    { -width / 2, -height / 2, -depth / 2 }, { -width / 2, -height / 2, depth / 2 },
                                    { -width / 2, height / 2, depth / 2 },   { -width / 2, height / 2, -depth / 2 },
                                    { width / 2, -height / 2, depth / 2 },   { width / 2, -height / 2, -depth / 2 },
                                    { width / 2, height / 2, -depth / 2 },   { width / 2, height / 2, depth / 2 },
                                    { -width / 2, height / 2, depth / 2 },   { width / 2, height / 2, depth / 2 },
                                    { width / 2, height / 2, -depth / 2 },   { -width / 2, height / 2, -depth / 2 },
                                    { -width / 2, -height / 2, -depth / 2 }, { width / 2, -height / 2, -depth / 2 },
                                    { width / 2, -height / 2, depth / 2 },   { -width / 2, -height / 2, depth / 2 } };

    const glm::vec3 normals[] = { { 0, 0, 1 }, { 0, 0, -1 }, { -1, 0, 0 }, { 1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 } };

    constexpr glm::vec2 uvs[] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };

    for (int face = 0; face < 6; ++face) {
        for (int vert = 0; vert < 4; ++vert) {
            Vertex v;
            v.position       = positions[face * 4 + vert];
            v.normal         = normals[face];
            v.texture_coords = uvs[vert];
            vertices.push_back(v);
        }
    }

    for (int face = 0; face < 6; ++face) {
        unsigned int base = face * 4;
        indices.insert(indices.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
    }

    calculate_tangents(vertices, indices);

    auto mesh = std::make_shared<Mesh>(vertices, indices, textures);
    std::vector<std::shared_ptr<Mesh>> meshes;
    meshes.push_back(mesh);
    return std::make_shared<Model>("internal://primitive/cube", meshes);
}

std::shared_ptr<Model> PrimitiveGenerator::generate_sphere(const std::unordered_map<std::string, std::any> &params) {
    const float radius = params.contains("radius") ? std::any_cast<float>(params.at("radius")) : 1.0f;
    const int sectors  = params.contains("sectors") ? std::any_cast<int>(params.at("sectors")) : 36;
    const int stacks   = params.contains("stacks") ? std::any_cast<int>(params.at("stacks")) : 18;

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<std::shared_ptr<Texture>> textures;

    try {
        if (params.contains("material")) {
            const auto &material = std::any_cast<std::unordered_map<std::string, std::any>>(params.at("material"));
            const auto color_r   = material.contains("color_r") ? std::any_cast<float>(material.at("color_r")) : 1.0f;
            const auto color_g   = material.contains("color_g") ? std::any_cast<float>(material.at("color_g")) : 1.0f;
            const auto color_b   = material.contains("color_b") ? std::any_cast<float>(material.at("color_b")) : 1.0f;
            const auto color_a   = material.contains("color_a") ? std::any_cast<float>(material.at("color_a")) : 1.0f;
            const auto type = material.contains("type") ? std::any_cast<TextureType>(material.at("type")) : EDiffuse;
            textures.push_back(Texture::create_solid_color(color_r, color_g, color_b, color_a, type));
        }
    } catch (const std::bad_any_cast &e) {
        get_logger()->error("Invalid material parameters: {}", e.what());
    }

    const auto PI = static_cast<float>(std::acos(-1));
    float x, y, z, xy;
    float nx, ny, nz;
    float s, t;

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = PI / 2 - static_cast<float>(i) * (PI / static_cast<float>(stacks));
        xy               = radius * cosf(stackAngle);
        z                = radius * sinf(stackAngle);

        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = static_cast<float>(j) * 2 * PI / static_cast<float>(sectors);

            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);

            nx = x / radius;
            ny = y / radius;
            nz = z / radius;

            s = static_cast<float>(j) / static_cast<float>(sectors);
            t = static_cast<float>(i) / static_cast<float>(stacks);

            Vertex v;
            v.position       = { x, y, z };
            v.normal         = { nx, ny, nz };
            v.texture_coords = { s, t };
            vertices.push_back(v);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        uint32_t k1 = i * (sectors + 1);
        uint32_t k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.insert(indices.end(), { k1, k2, k1 + 1 });
            }
            if (i != stacks - 1) {
                indices.insert(indices.end(), { k1 + 1, k2, k2 + 1 });
            }
        }
    }

    calculate_tangents(vertices, indices);

    auto mesh  = std::make_shared<Mesh>(vertices, indices, textures);
    auto model = std::make_shared<Model>("internal://primitive/sphere", std::vector{ mesh });
    return model;
}

std::shared_ptr<Model> PrimitiveGenerator::generate_plane(const std::unordered_map<std::string, std::any> &params) {
    const float width    = params.contains("width") ? std::any_cast<float>(params.at("width")) : 1.0f;
    const float height   = params.contains("height") ? std::any_cast<float>(params.at("height")) : 1.0f;
    const int segments_x = params.contains("segments_x") ? std::any_cast<int>(params.at("segments_x")) : 1;
    const int segments_z = params.contains("segments_z") ? std::any_cast<int>(params.at("segments_z")) : 1;

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<std::shared_ptr<Texture>> textures;

    try {
        if (params.contains("material")) {
            const auto &material = std::any_cast<std::unordered_map<std::string, std::any>>(params.at("material"));
            const auto color_r   = material.contains("color_r") ? std::any_cast<float>(material.at("color_r")) : 1.0f;
            const auto color_g   = material.contains("color_g") ? std::any_cast<float>(material.at("color_g")) : 1.0f;
            const auto color_b   = material.contains("color_b") ? std::any_cast<float>(material.at("color_b")) : 1.0f;
            const auto color_a   = material.contains("color_a") ? std::any_cast<float>(material.at("color_a")) : 1.0f;
            const auto type = material.contains("type") ? std::any_cast<TextureType>(material.at("type")) : EDiffuse;
            textures.push_back(Texture::create_solid_color(color_r, color_g, color_b, color_a, type));
        }
    } catch (const std::bad_any_cast &e) {
        get_logger()->error("Invalid material parameters: {}", e.what());
    }

    const float x_step    = width / static_cast<float>(segments_x);
    const float z_step    = height / static_cast<float>(segments_z);
    const float uv_x_step = 1.0f / static_cast<float>(segments_x);
    const float uv_z_step = 1.0f / static_cast<float>(segments_z);

    for (int z = 0; z <= segments_z; ++z) {
        for (int x = 0; x <= segments_x; ++x) {
            Vertex v;
            v.position       = { -width / 2 + static_cast<float>(x) * x_step, 0.0f,
                                 -height / 2 + static_cast<float>(z) * z_step };
            v.normal         = { 0.0f, 1.0f, 0.0f };
            v.texture_coords = { static_cast<float>(x) * uv_x_step, 1.0f - static_cast<float>(z) * uv_z_step };
            vertices.push_back(v);
        }
    }

    for (int z = 0; z < segments_z; ++z) {
        for (int x = 0; x < segments_x; ++x) {
            uint32_t lt = z * (segments_x + 1) + x;
            uint32_t rt = lt + 1;
            uint32_t lb = lt + (segments_x + 1);
            uint32_t rb = lb + 1;

            indices.insert(indices.end(), { lt, rt, lb });
            indices.insert(indices.end(), { lb, rt, rb });
        }
    }

    calculate_tangents(vertices, indices);

    auto mesh = std::make_shared<Mesh>(vertices, indices, textures);
    return std::make_shared<Model>("internal://primitive/plane", std::vector{ mesh });
}

void PrimitiveGenerator::calculate_tangents(std::vector<Vertex> &vertices, const std::vector<GLuint> &indices) {
    std::vector tan1(vertices.size(), glm::vec3(0));
    std::vector tan2(vertices.size(), glm::vec3(0));

    for (size_t i = 0; i < indices.size(); i += 3) {
        const auto i1 = indices[i];
        const auto i2 = indices[i + 1];
        const auto i3 = indices[i + 2];

        const auto &v1 = vertices[i1];
        const auto &v2 = vertices[i2];
        const auto &v3 = vertices[i3];

        const glm::vec3 edge1 = v2.position - v1.position;
        const glm::vec3 edge2 = v3.position - v1.position;

        const glm::vec2 deltaUV1 = v2.texture_coords - v1.texture_coords;
        const glm::vec2 deltaUV2 = v3.texture_coords - v1.texture_coords;

        const float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent   = normalize(tangent);

        tan1[i1] += tangent;
        tan1[i2] += tangent;
        tan1[i3] += tangent;
    }

    for (size_t i = 0; i < vertices.size(); ++i) {
        auto &v            = vertices[i];
        const glm::vec3 &n = v.normal;
        const glm::vec3 &t = tan1[i];

        v.tangent    = normalize(t - n * dot(n, t));
        v.bi_tangent = cross(n, v.tangent);
    }
}

std::shared_ptr<Model> PrimitiveGenerator::generate(const std::string &type,
                                                    const std::unordered_map<std::string, std::any> &params) {
    static const std::unordered_map<
        std::string, std::function<std::shared_ptr<Model>(const std::unordered_map<std::string, std::any> &)>>
        generators = { { "cube", generate_cube }, { "sphere", generate_sphere }, { "plane", generate_plane } };
    if (auto it = generators.find(type); it != generators.end()) {
        return it->second(params);
    }
    return nullptr;
}
