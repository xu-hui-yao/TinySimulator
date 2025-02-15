#pragma once

#include <glm\glm.hpp>

enum LightType {
    Point,
    Directional,
};

struct PointLightComponent {
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    LightType type = Point;
};

struct DirectionalLightComponent {
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    glm::vec3 direction = {0.0f, 0.0f, 1.0f};
    LightType type = Directional;
};