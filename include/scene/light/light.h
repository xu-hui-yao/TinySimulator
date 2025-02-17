#pragma once

#include <glm/glm.hpp>

enum LightType {
    e_unknown,
    e_point,
    e_directional,
};

class Light {
protected:
    LightType m_type{e_unknown};
};

class PointLight : public Light {
private:
    glm::vec3 m_position{};
    glm::vec3 m_intensity{};
};

class DirectionalLight : public Light {
private:
    glm::vec3 m_direction{};
    glm::vec3 m_intensity{};
};