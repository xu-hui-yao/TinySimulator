#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>

struct Transform {
    glm::vec3 position{ 0.0f };
    glm::vec3 rotation{ 0.0f };  // Euler angle
    glm::vec3 scale{ 1.0f };

    [[nodiscard]] glm::mat3 orientation_matrix() const {
        return glm::eulerAngleXYZ(rotation.x, rotation.y, rotation.z);
    }

    [[nodiscard]] glm::mat4 matrix() const {
        return glm::translate(glm::mat4(1.0f), position) * glm::orientate4(rotation) *
               glm::scale(glm::mat4(1.0f), scale);
    }
};
