#include <ecs/component/cloth.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <set>

Cloth::Cloth(const std::shared_ptr<Model> &model, const Transform &transform, float density) : model(model) {
    if (!model || model->get_meshes().size() > 1) {
        get_logger()->error("Model is null or does not have a single mesh");
        return;
    }

    model->unload();
    model->get_vertices_changed() = true;
    model->upload(nullptr);

    init_transform = transform.matrix();

    auto vertices = model->get_meshes()[0]->get_vertices();
    for (const auto &vertex : vertices) {
        positions.emplace_back(init_transform * glm::vec4(vertex.position, 1.0f));
    }

    auto size = positions.size();
    pred_positions.resize(size);
    velocities.resize(size);
    inv_masses.resize(size);
    fixed_vertices.resize(size);

    std::ranges::copy(positions.begin(), positions.end(), pred_positions.begin());
    std::ranges::fill(velocities.begin(), velocities.end(), glm::vec3(0.0f, 0.0f, 0.0f));
    std::ranges::fill(inv_masses.begin(), inv_masses.end(), 0.0f);
    std::fill(fixed_vertices.begin(), fixed_vertices.end(), false);

    indices           = model->get_meshes()[0]->get_indices();
    auto indices_size = indices.size();
    for (int i = 0; i < indices_size; i += 3) {
        auto i0                 = indices[i];
        auto i1                 = indices[i + 1];
        auto i2                 = indices[i + 2];
        auto v0                 = positions[i0];
        auto v1                 = positions[i1];
        auto v2                 = positions[i2];
        glm::vec3 edge1         = v1 - v0;
        glm::vec3 edge2         = v2 - v0;
        glm::vec3 cross_product = glm::cross(edge1, edge2);
        float area              = 0.5f * glm::length(cross_product);
        float avg_mass          = area * density / 3.0f;
        inv_masses[i0] += avg_mass;
        inv_masses[i1] += avg_mass;
        inv_masses[i2] += avg_mass;
    }

    for (int i = 0; i < vertices.size(); i++) {
        inv_masses[i] = 1.0f / std::max(inv_masses[i], 1e-4f);
    }

    auto distance_constraint = std::make_shared<DistanceConstraint>(*this);
    auto bend_constraint     = std::make_shared<BendConstraint>(*this);
    constraints.emplace_back(distance_constraint);
    constraints.emplace_back(bend_constraint);
}

void Cloth::update_model(const Transform &transform) const {
    glm::mat4 inverse = glm::inverse(transform.matrix());
    auto &vertices    = model->get_meshes()[0]->get_vertices();
    for (size_t i = 0; i < positions.size(); ++i) {
        vertices[i].position = inverse * glm::vec4(positions[i], 1.0f);
    }
    model->update_gpu_buffer();
}

DistanceConstraint::DistanceConstraint(const Cloth &cloth) {
    std::set<std::pair<int, int>> unique_edges;
    for (size_t i = 0; i < cloth.indices.size(); i += 3) {
        GLuint tri[3] = { cloth.indices[i], cloth.indices[i + 1], cloth.indices[i + 2] };
        for (int j = 0; j < 3; ++j) {
            auto a = tri[j], b = tri[(j + 1) % 3];
            if (a > b)
                std::swap(a, b);
            if (unique_edges.insert({ a, b }).second) {
                auto p_a = glm::vec3(cloth.init_transform * glm::vec4(cloth.positions[a], 1.0f));
                auto p_b = glm::vec3(cloth.init_transform * glm::vec4(cloth.positions[b], 1.0f));
                constraints.push_back({ static_cast<int>(a), static_cast<int>(b), glm::distance(p_a, p_b) });
            }
        }
    }
}

void DistanceConstraint::project(Cloth &cloth, int iterations) {
    for (int it = 0; it < iterations; ++it) {
        for (auto &c : constraints) {
            glm::vec3 &p0           = cloth.pred_positions[c.v0];
            glm::vec3 &p1           = cloth.pred_positions[c.v1];
            float w0                = cloth.inv_masses[c.v0];
            float w1                = cloth.inv_masses[c.v1];
            glm::vec3 delta         = p1 - p0;
            float length            = glm::length(delta);
            constexpr float epsilon = 1e-4f;

            if (length < epsilon) {
                continue;
            }

            // Adjust rest length by current scale
            float constraint_value = length - c.rest_length;
            glm::vec3 dir          = delta / length;

            // Apply correction in world space
            glm::vec3 correction = constraint_value * dir * (cloth.distance_stiffness / static_cast<float>(iterations));
            if (!cloth.fixed_vertices[c.v0] && !cloth.fixed_vertices[c.v1]) {
                p0 += correction * (w0 / (w0 + w1));
                p1 -= correction * (w1 / (w0 + w1));
            } else if (!cloth.fixed_vertices[c.v0] && cloth.fixed_vertices[c.v1]) {
                p0 += correction;
            } else if (cloth.fixed_vertices[c.v0] && !cloth.fixed_vertices[c.v1]) {
                p1 -= correction;
            }
        }
    }
}

BendConstraint::BendConstraint(const Cloth &cloth) {
    // Edge - triangle id map
    std::map<std::pair<int, int>, std::vector<int>> edge_tri_map;
    for (size_t i = 0; i < cloth.indices.size(); i += 3) {
        int tri_idx   = static_cast<int>(i) / 3;
        GLuint tri[3] = { cloth.indices[i], cloth.indices[i + 1], cloth.indices[i + 2] };
        for (int j = 0; j < 3; ++j) {
            int a = static_cast<int>(tri[j]);
            int b = static_cast<int>(tri[(j + 1) % 3]);
            if (a > b)
                std::swap(a, b);
            edge_tri_map[{ a, b }].push_back(tri_idx);
        }
    }

    // Find shared edges
    std::set<std::tuple<int, int, int, int>> processed_quads;
    for (const auto &[edge, tri_indices] : edge_tri_map) {
        if (tri_indices.size() != 2) {
            continue;
        }
        const int tri1_idx = tri_indices[0];
        const int tri2_idx = tri_indices[1];
        const auto *tri1   = &cloth.indices[tri1_idx * 3];
        const auto *tri2   = &cloth.indices[tri2_idx * 3];
        // Find vertex not on the shared edge
        auto find_third_vertex = [](const GLuint *tri, int a, int b) {
            for (int i = 0; i < 3; ++i) {
                if (tri[i] != a && tri[i] != b) {
                    return tri[i];
                }
            }
            return tri[0];
        };
        const int a = edge.first;
        const int b = edge.second;
        const int c = static_cast<int>(find_third_vertex(tri1, a, b));
        const int d = static_cast<int>(find_third_vertex(tri2, a, b));
        // Make sure it is unique
        auto quad = std::make_tuple(std::min(a, b), std::max(a, b), std::min(c, d), std::max(c, d));
        if (processed_quads.contains(quad))
            continue;
        processed_quads.insert(quad);

        // Calculate normal
        auto calc_normal = [&](int v0, int v1, int v2) {
            glm::vec3 p0 = cloth.positions[v0];
            glm::vec3 p1 = cloth.positions[v1];
            glm::vec3 p2 = cloth.positions[v2];
            return glm::normalize(glm::cross(p1 - p0, p2 - p0));
        };
        glm::vec3 n1 = calc_normal(a, b, c);
        glm::vec3 n2 = calc_normal(a, b, d);

        // Calculate angle
        float cos_theta  = glm::dot(n1, n2);
        cos_theta        = glm::clamp(cos_theta, -1.0f, 1.0f);
        float rest_angle = glm::acos(cos_theta);

        // Add constraint
        constraints.push_back({ a, b, c, d, rest_angle });
    }
}

void BendConstraint::project(Cloth &cloth, int iterations) {
    for (int it = 0; it < iterations; ++it) {
        for (auto &c : constraints) {
            constexpr float epsilon = 1e-4f;
            auto p1                 = cloth.pred_positions[c.a];
            auto p2                 = cloth.pred_positions[c.b];
            auto p3                 = cloth.pred_positions[c.c];
            auto p4                 = cloth.pred_positions[c.d];
            p2 -= p1;
            p3 -= p1;
            p4 -= p1;
            auto n1 = glm::normalize(glm::cross(p2, p3));
            auto n2 = glm::normalize(glm::cross(p2, p4));
            auto d  = glm::clamp(glm::dot(n1, n2), -1.0f, 1.0f);
            auto q3 =
                (glm::cross(p2, n2) + d * (glm::cross(n1, p2))) / std::max(glm::length(glm::cross(p2, p3)), epsilon);
            auto q4 =
                (glm::cross(p2, n1) + d * (glm::cross(n2, p2))) / std::max(glm::length(glm::cross(p2, p4)), epsilon);
            auto q2 =
                -(glm::cross(p3, n2) + d * (glm::cross(n1, p3))) / std::max(glm::length(glm::cross(p2, p3)), epsilon) -
                (glm::cross(p4, n1) + d * (glm::cross(n2, p4))) / std::max(glm::length(glm::cross(p2, p4)), epsilon);
            auto q1 = -q2 - q3 - q4;
            auto denom =
                (cloth.fixed_vertices[c.a] ? 0.0f : cloth.inv_masses[c.a] * glm::length(q1) * glm::length(q1)) +
                (cloth.fixed_vertices[c.b] ? 0.0f : cloth.inv_masses[c.b] * glm::length(q2) * glm::length(q2)) +
                (cloth.fixed_vertices[c.c] ? 0.0f : cloth.inv_masses[c.c] * glm::length(q3) * glm::length(q3)) +
                (cloth.fixed_vertices[c.d] ? 0.0f : cloth.inv_masses[c.d] * glm::length(q4) * glm::length(q4));
            if (denom < epsilon) {
                continue;
            }
            auto numerator = std::sqrt(1.0f - d * d) * (glm::acos(d) - c.rest_angle) *
                             (cloth.bend_stiffness / static_cast<float>(iterations));
            cloth.pred_positions[c.a] -=
                cloth.fixed_vertices[c.a] ? glm::vec3(0.0f) : cloth.inv_masses[c.a] * q1 * numerator / denom;
            cloth.pred_positions[c.b] -=
                cloth.fixed_vertices[c.b] ? glm::vec3(0.0f) : cloth.inv_masses[c.b] * q2 * numerator / denom;
            cloth.pred_positions[c.c] -=
                cloth.fixed_vertices[c.c] ? glm::vec3(0.0f) : cloth.inv_masses[c.c] * q3 * numerator / denom;
            cloth.pred_positions[c.d] -=
                cloth.fixed_vertices[c.d] ? glm::vec3(0.0f) : cloth.inv_masses[c.d] * q4 * numerator / denom;
        }
    }
}
