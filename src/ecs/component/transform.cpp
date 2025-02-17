#include <ecs/component/transform.h>
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Transform::matrix() const {
    glm::mat4 model(1.0f);
    model = glm::translate(model, m_position);
    model = glm::rotate(model, glm::radians(m_rotation.x), {1,0,0});
    model = glm::rotate(model, glm::radians(m_rotation.y), {0,1,0});
    model = glm::rotate(model, glm::radians(m_rotation.z), {0,0,1});
    return glm::scale(model, m_scale);
}
