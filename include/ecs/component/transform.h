#pragma once

#include <glm/glm.hpp>

struct Transform {
    // Spatial transformation parameters
    glm::vec3 position{ 0.0f };
    glm::vec3 rotation{ 0.0f }; // Euler angles (in radians)
    glm::vec3 scale{ 1.0f };

    explicit Transform(const glm::vec3 &pos = glm::vec3(0.0f), const glm::vec3 &rot = glm::vec3(0.0f),
                       const glm::vec3 &scl = glm::vec3(1.0f));

    // Get the orientation quaternion (automatically cached)
    [[nodiscard]] glm::quat orientation() const;

    // Generate the model matrix
    [[nodiscard]] glm::mat4 matrix() const;
};
