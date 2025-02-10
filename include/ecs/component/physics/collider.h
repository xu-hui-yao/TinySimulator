#pragma once

#include <glm\glm.hpp>

enum ColliderType {
    Box,
    Sphere,
    Capsule
};

struct ColliderComponent {
    ColliderType type = Box; // 盒子、球体、胶囊等
    glm::vec3 size = {1.0f, 1.0f, 1.0f}; // 碰撞体尺寸
};