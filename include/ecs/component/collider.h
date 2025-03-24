#pragma once

#include <glm/glm.hpp>

// Collider component with multiple shape support
struct Collider {
    enum ShapeType { SPHERE, BOX, CAPSULE } shape_type;

    // Local offset from transform position
    glm::vec3 offset = { 0.0f, 0.0f, 0.0f };

    // Shape parameters union to save memory
    union {
        struct {
            float radius; // Sphere radius
        };
        struct {
            glm::vec3 half_extents; // Box half dimensions (x/2, y/2, z/2)
        };
        struct {
            float capsule_radius; // Capsule base radius
            float capsule_height; // Capsule total height (along local Y-axis)
        };
    };

    // Collision flags
    bool is_trigger = false; // Trigger flag (no physical response)
    bool is_active  = true;  // Collision detection enable flag
};
