#include <ecs/component/transform.h>
#include <glm/gtx/quaternion.hpp>

Transform::Transform(const glm::vec3 &pos, const glm::vec3 &rot, const glm::vec3 &scl)
    : position(pos), rotation(rot), scale(scl) {}

glm::quat Transform::orientation() const { return { rotation }; }

glm::mat4 Transform::matrix() const {
    return glm::translate(glm::mat4(1.0f), position) * glm::toMat4(orientation()) * glm::scale(glm::mat4(1.0f), scale);
}