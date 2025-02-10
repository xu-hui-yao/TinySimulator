#pragma once

#include <glm/glm.hpp>

struct CameraComponent {
    float fov        = 45.0f;
    float near_plane = 0.1f;
    float far_plane  = 100.0f;
    glm::mat4 view_matrix{};
    glm::mat4 projection_matrix{};
    bool is_main_camera = false;
};