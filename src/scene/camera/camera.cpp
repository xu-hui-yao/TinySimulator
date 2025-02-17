#include <glm/gtc/matrix_transform.hpp>
#include <scene/camera/camera.h>

Camera::Camera(const glm::vec3 &position, const glm::vec3 &lookat, const glm::vec3 &up, float fov, float aspect,
               float near, float far)
    : m_position(position), world_up(up), m_fov(fov), m_aspect(aspect), m_near(near), m_far(far) {
    look_at(lookat);
}

glm::mat4 Camera::get_view_matrix() const { return glm::lookAt(m_position, m_position + m_forward, m_up); }

glm::mat4 Camera::get_projection_matrix() const {
    return glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
}

// Position methods
void Camera::set_position(const glm::vec3 &position) { m_position = position; }
void Camera::offset_position(const glm::vec3 &offset) { m_position += offset; }
const glm::vec3 &Camera::get_position() const { return m_position; }

// Orientation methods
void Camera::look_at(const glm::vec3 &target) {
    m_forward = glm::normalize(target - m_position);
    update_vectors();
}

const glm::vec3 &Camera::get_forward() const { return m_forward; }
const glm::vec3 &Camera::get_up() const { return m_up; }
const glm::vec3 &Camera::get_right() const { return m_right; }

// Update internal vectors when orientation changes
void Camera::update_vectors() {
    m_right = glm::normalize(glm::cross(m_forward, world_up));
    m_up    = glm::normalize(glm::cross(m_right, m_forward));
}

// Projection control
void Camera::set_perspective(float fov, float aspect, float near, float far) {
    m_fov    = fov;
    m_aspect = aspect;
    m_near   = near;
    m_far    = far;
}

float Camera::get_fov() const { return m_fov; }
float Camera::get_aspect() const { return m_aspect; }
float Camera::get_near() const { return m_near; }
float Camera::get_far() const { return m_far; }
