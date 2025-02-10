#pragma once

#include <glm\glm.hpp>

struct RigidBodyComponent {
    float mass = 1.0f;
    glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
    glm::vec3 angular_velocity = {0.0f, 0.0f, 0.0f};
    bool is_kinematic = false; // 是否为运动学刚体
};