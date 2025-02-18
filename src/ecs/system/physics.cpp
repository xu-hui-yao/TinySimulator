#include <ecs/system/physics.h>
#include <ecs/component/transform.h>
#include <ecs/component/rigidbody.h>

void PhysicsSystem::update(entt::registry& registry, float delta_time) {
    constexpr glm::vec3 gravity{0.0f, -9.81f, 0.0f};

    registry.view<Transform, RigidBody>().each(
        [&](auto& transform, auto& rb) {
            // Apply gravity
            if (rb.m_use_gravity) {
                rb.m_velocity += gravity * delta_time;
            }

            // Update position
            transform.m_position += rb.m_velocity * delta_time;

            // Simple ground collision
            if (transform.m_position.y < 0.0f) {
                transform.m_position.y = 0.0f;
                rb.m_velocity.y = 0.0f;
            }
        }
    );
}
