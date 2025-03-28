#include <glm/gtc/matrix_transform.hpp>
#include <scene/camera/camera.h>

Camera::Camera(const glm::vec3 &position, const glm::vec3 &lookat, const glm::vec3 &up, float fov, float aspect,
               float near, float far, float speed, float sensitivity, bool fix)
    : m_position(position), m_world_up(up), m_fov(fov), m_aspect(aspect), m_near(near), m_far(far), m_speed(speed),
      m_sensitivity(sensitivity), fix(fix) {
    look_at(lookat);
    set_perspective(fov, aspect, near, far);
}

glm::mat4 Camera::get_view_matrix() const { return glm::lookAt(m_position, m_position + m_forward, m_up); }

glm::mat4 Camera::get_projection_matrix() const { return m_projection_matrix; }

void Camera::update_orientation(const glm::vec2 &offset) {
    if (fix) {
        return;
    }

    m_yaw += offset.x * m_sensitivity;
    m_pitch -= offset.y * m_sensitivity;

    m_pitch = glm::clamp(m_pitch, -89.9f, 89.9f);

    m_forward =
        glm::normalize(glm::vec3(cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch)), sin(glm::radians(m_pitch)),
                                 sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch))));

    update_vectors();
}

// Position methods
void Camera::set_position(const glm::vec3 &position) { m_position = position; }
void Camera::offset_position(const glm::vec3 &offset) { m_position += offset; }
const glm::vec3 &Camera::get_position() const { return m_position; }
void Camera::set_speed(float speed) { m_speed = speed; }
void Camera::offset_speed(float speed) { m_speed += speed; }
float Camera::get_speed() const { return m_speed; }
void Camera::set_sensitivity(float sensitivity) { m_sensitivity = sensitivity; }
float Camera::get_sensitivity() const { return m_sensitivity; }

// Orientation methods
void Camera::look_at(const glm::vec3 &target) {
    glm::vec3 direction = target - m_position;
    m_forward           = glm::length(direction) > 0.0f ? glm::normalize(direction) : glm::vec3(0.0f, 0.0f, -1.0f);
    m_pitch             = asin(m_forward.y);
    m_yaw               = glm::degrees(atan2(m_forward.z, m_forward.x));
    update_vectors();
}

const glm::vec3 &Camera::get_forward() const { return m_forward; }
const glm::vec3 &Camera::get_up() const { return m_up; }
const glm::vec3 &Camera::get_world_up() const { return m_world_up; }
const glm::vec3 &Camera::get_right() const { return m_right; }

// Update internal vectors when orientation changes
void Camera::update_vectors() {
    m_right = glm::normalize(glm::cross(m_forward, m_world_up));
    m_up    = glm::normalize(glm::cross(m_right, m_forward));
}

// Projection control
void Camera::set_perspective(float fov, float aspect, float near, float far) {
    m_fov               = fov;
    m_aspect            = aspect;
    m_near              = near;
    m_far               = far;
    m_projection_matrix = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
}

float Camera::get_fov() const { return m_fov; }
float Camera::get_aspect() const { return m_aspect; }
float Camera::get_near() const { return m_near; }
float Camera::get_far() const { return m_far; }

void Camera::move_forward(float delta_time) { offset_position(m_forward * m_speed * delta_time); }

void Camera::move_backward(float delta_time) { offset_position(-m_forward * m_speed * delta_time); }

void Camera::move_left(float delta_time) { offset_position(-m_right * m_speed * delta_time); }

void Camera::move_right(float delta_time) { offset_position(m_right * m_speed * delta_time); }

void Camera::move_up(float delta_time) { offset_position(m_world_up * m_speed * delta_time); }

void Camera::move_down(float delta_time) { offset_position(-m_world_up * m_speed * delta_time); }
