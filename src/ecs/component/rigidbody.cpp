#include <ecs/component/rigidbody.h>

void RigidBody::set_mass(float new_mass) {
    mass = new_mass;
    update_inertia();
}

void RigidBody::set_inertia_tensor(const glm::mat3 &new_inertia) {
    inertia_tensor = new_inertia;
    update_inertia();
}

void RigidBody::update_inertia() {
    if (mass <= 0.0f || glm::determinant(inertia_tensor) == 0.0f) {
        inv_inertia_tensor = glm::mat3(0.0f);
    } else {
        inv_inertia_tensor = glm::inverse(inertia_tensor);
    }
}

void RigidBody::integrate(float dt, const glm::vec3 &gravity) {
    if (is_kinematic || mass <= 0.0f)
        return;

    // Apply gravity
    if (use_gravity) {
        force_accumulator += gravity * mass;
    }

    // Linear integration
    const glm::vec3 acceleration = force_accumulator / mass;
    linear_velocity += acceleration * dt;
    linear_velocity *= glm::max(1.0f - linear_damping * dt, 0.0f);

    // Angular integration
    const glm::vec3 angular_acceleration = inv_inertia_tensor * torque_accumulator;
    angular_velocity += angular_acceleration * dt;
    angular_velocity *= glm::max(1.0f - angular_damping * dt, 0.0f);

    // Apply constraints
    linear_velocity *= glm::vec3(1) - glm::vec3(freeze_position);
    angular_velocity *= glm::vec3(1) - glm::vec3(freeze_rotation);

    // Reset accumulators
    force_accumulator  = glm::vec3(0);
    torque_accumulator = glm::vec3(0);
}

void RigidBody::add_force(const glm::vec3 &force) { force_accumulator += force; }

void RigidBody::add_force_at_position(const glm::vec3 &force, const glm::vec3 &position) {
    force_accumulator += force;
    torque_accumulator += glm::cross(position - center_of_mass, force);
}

void RigidBody::add_torque(const glm::vec3 &torque) { torque_accumulator += torque; }
