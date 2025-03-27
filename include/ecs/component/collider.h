#pragma once

#include <glm/glm.hpp>
#include <scene/model/model.h>

struct Collider {
    enum ShapeType { SPHERE, BOX, CAPSULE } shape;

    // Common properties
    glm::vec3 offset{ 0.0f };  // Local offset from entity's transform
    bool is_trigger   = false; // Trigger flag (no physical response)
    bool is_active    = true;  // Collision detection toggle
    float friction    = 0.5f;  // Surface friction coefficient (0-1)
    float restitution = 0.2f;  // Bounce coefficient (0=inelastic, 1=perfect bounce)
    union {
        struct {
            float radius;
        }; // Sphere parameters
        struct {
            glm::vec3 half_extents;
        }; // Box parameters (half sizes)
        struct {
            float capsule_radius, capsule_height;
        }; // Capsule parameters
    };
    bool visualize = false;
    std::shared_ptr<Model> visualize_model;

    void generate_visualize_model();
};
