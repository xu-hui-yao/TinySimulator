#pragma once

#include <glm/glm.hpp>

struct Transform {
    glm::vec3 m_position{0.0f};
    glm::vec3 m_rotation{0.0f}; // Euler angles
    glm::vec3 m_scale{1.0f};

    [[nodiscard]] glm::mat4 matrix() const;
};