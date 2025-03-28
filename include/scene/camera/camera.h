#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    explicit Camera(const glm::vec3 &position = glm::vec3(0.0f), const glm::vec3 &lookat = glm::vec3(0.0f, 0.0f, -1.0f),
                    const glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f), float fov = 45.0f, float aspect = 16.0f / 9.0f,
                    float near = 0.1f, float far = 100.0f, float speed = 5.0f, float sensitivity = 0.08f, bool fix = false);

    [[nodiscard]] glm::mat4 get_view_matrix() const;
    [[nodiscard]] glm::mat4 get_projection_matrix() const;

    void update_orientation(const glm::vec2 &offset);

    // Position control
    void set_position(const glm::vec3 &position);
    void offset_position(const glm::vec3 &offset);
    [[nodiscard]] const glm::vec3 &get_position() const;
    void set_speed(float speed);
    void offset_speed(float speed);
    [[nodiscard]] float get_speed() const;
    void set_sensitivity(float sensitivity);
    [[nodiscard]] float get_sensitivity() const;

    // Orientation control
    void look_at(const glm::vec3 &target);
    [[nodiscard]] const glm::vec3 &get_forward() const;
    [[nodiscard]] const glm::vec3 &get_up() const;
    [[nodiscard]] const glm::vec3 &get_world_up() const;
    [[nodiscard]] const glm::vec3 &get_right() const;

    // Projection control
    void set_perspective(float fov, float aspect, float near, float far);
    [[nodiscard]] float get_fov() const;
    [[nodiscard]] float get_aspect() const;
    [[nodiscard]] float get_near() const;
    [[nodiscard]] float get_far() const;

    // Move
    void move_forward(float delta_time);
    void move_backward(float delta_time);
    void move_left(float delta_time);
    void move_right(float delta_time);
    void move_up(float delta_time);
    void move_down(float delta_time);

private:
    void update_vectors();

    glm::vec3 m_position{};
    glm::vec3 m_forward{};
    glm::vec3 m_up{};
    glm::vec3 m_right{};
    glm::vec3 m_world_up{};

    glm::mat4 m_projection_matrix{};

    float m_fov;
    float m_aspect;
    float m_near;
    float m_far;

    float m_speed;
    float m_sensitivity;

    float m_yaw{};
    float m_pitch{};

    bool fix{};
};
