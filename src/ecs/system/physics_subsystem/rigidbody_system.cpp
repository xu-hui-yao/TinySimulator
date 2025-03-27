#include <ecs/component/rigidbody.h>
#include <ecs/component/transform.h>
#include <ecs/system/physics_subsystem/rigidbody_system.h>
#include <glm/gtc/quaternion.hpp>

int RigidBodySystem::execution_priority() const { return priority; }

void RigidBodySystem::update(entt::registry &registry, float dt) { integrate_forces(registry, dt); }

void RigidBodySystem::integrate_forces(entt::registry &registry, float dt) {
    auto view = registry.view<Transform, RigidBody>();
    view.each([dt, this](auto &t, auto &rb) {
        if (rb.is_kinematic || rb.mass <= 0.0f)
            return;
        rb.integrate(dt, gravity);
        t.position += rb.linear_velocity * dt;
        // Quaternion integration with proper angular velocity handling
        glm::quat rotation = t.orientation();
        const glm::quat angular_quat(0, rb.angular_velocity.x, rb.angular_velocity.y, rb.angular_velocity.z);
        rotation   = glm::normalize(rotation + 0.5f * dt * angular_quat * rotation);
        t.rotation = glm::eulerAngles(rotation);
    });
}
