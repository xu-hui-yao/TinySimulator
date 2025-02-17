#pragma once
#include <glm/glm.hpp>

struct RigidBody {
    glm::vec3 m_velocity{0.0f};
    float m_mass = 1.0f;
    bool m_use_gravity = true;
};