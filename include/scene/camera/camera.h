#pragma once

#include <glm/glm.hpp>

class Camera {
private:
    glm::vec3 m_position{};
    glm::vec3 m_forward{};
    glm::vec3 m_up{};
    glm::vec3 m_right{};
    glm::vec3 world_up{0, 1, 0};
    float m_near{};
    float m_far{};
};