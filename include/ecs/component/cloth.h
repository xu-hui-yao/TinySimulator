#pragma once

#include <ecs/component/transform.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <scene/model/model.h>
#include <vector>

struct Cloth;

class Constraint {
public:
    virtual ~Constraint() = default;

    virtual void project(Cloth &cloth, int iterations) = 0;
};

class DistanceConstraint : public Constraint {
public:
    struct ConstraintData {
        int v0, v1;
        float rest_length;
    };
    std::vector<ConstraintData> constraints;

    explicit DistanceConstraint(const Cloth &cloth);

    void project(Cloth &cloth, int iterations) override;
};

class BendConstraint : public Constraint {
public:
    struct ConstraintData {
        int a, b, c, d;
        float rest_angle;
    };
    std::vector<ConstraintData> constraints;

    explicit BendConstraint(const Cloth &cloth);

    void project(Cloth &cloth, int iterations) override;
};

struct Cloth {
    std::shared_ptr<Model> model;
    bool visualize = false;
    std::vector<glm::vec3> positions;
    std::vector<GLuint> indices;
    std::vector<glm::vec3> pred_positions;
    std::vector<glm::vec3> velocities;
    std::vector<float> inv_masses;
    std::vector<std::shared_ptr<Constraint>> constraints;
    std::vector<bool> fixed_vertices;
    float distance_stiffness = 0.9f;
    float bend_stiffness     = 0.3f;
    float friction_factor    = 0.1f;
    float damping            = 0.01f;
    glm::vec3 field_force    = glm::vec3(0.0f, 0.0f, -1e-2f);
    glm::vec3 gravity        = glm::vec3(0.0f, -9.81f, 0.0f);
    glm::mat4 init_transform = glm::identity<glm::mat4>();

    explicit Cloth(const std::shared_ptr<Model> &model, const Transform &transform, float density);

    void update_model(const Transform &transform) const;
};
