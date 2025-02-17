#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    explicit Camera(
        const glm::vec3& position = glm::vec3(0.0f),
        const glm::vec3& lookat = glm::vec3(0.0f, 0.0f, -1.0f),
        const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
        float fov = 45.0f,
        float aspect = 16.0f/9.0f,
        float near = 0.1f,
        float far = 100.0f
    );

    [[nodiscard]] glm::mat4 get_view_matrix() const;
    [[nodiscard]] glm::mat4 get_projection_matrix() const;

    // Position control
    void set_position(const glm::vec3& position);
    void offset_position(const glm::vec3& offset);
    [[nodiscard]] const glm::vec3& get_position() const;

    // Orientation control
    void look_at(const glm::vec3& target);
    [[nodiscard]] const glm::vec3& get_forward() const;
    [[nodiscard]] const glm::vec3& get_up() const;
    [[nodiscard]] const glm::vec3& get_right() const;

    // Projection control
    void set_perspective(float fov, float aspect, float near, float far);
    [[nodiscard]] float get_fov() const;
    [[nodiscard]] float get_aspect() const;
    [[nodiscard]] float get_near() const;
    [[nodiscard]] float get_far() const;

private:
    void update_vectors();

    glm::vec3 m_position{};
    glm::vec3 m_forward{};
    glm::vec3 m_up{};
    glm::vec3 m_right{};
    glm::vec3 world_up{};

    float m_fov;
    float m_aspect;
    float m_near;
    float m_far;
};
