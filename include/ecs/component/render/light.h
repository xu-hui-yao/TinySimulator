#pragma once

#include <glm\glm.hpp>

enum LightType {
    Directional,
    Point,
    Spot
};

struct LightComponent {
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    LightType type = Point;
};