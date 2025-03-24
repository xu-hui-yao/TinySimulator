#pragma once

#include <glm/glm.hpp>

/**
 * @brief Rigid body component for physical simulation
 *
 * Handles both linear and angular motion dynamics with proper force/torque
 * application and constraints.
 */
struct RigidBody {
    // Linear motion
    glm::vec3 linear_velocity{ 0.0f };
    glm::vec3 force_accumulator{ 0.0f };

    // Angular motion
    glm::vec3 angular_velocity{ 0.0f };
    glm::vec3 torque_accumulator{ 0.0f };

    // Physical properties
    float mass = 1.0f;
    glm::mat3 inertia_tensor{};
    glm::mat3 inv_inertia_tensor{};   // Precomputed inverse
    glm::vec3 center_of_mass{ 0.0f }; // Local space

    // Control parameters
    float linear_damping  = 0.01f;
    float angular_damping = 0.05f;
    bool use_gravity      = true;
    bool is_kinematic     = false;

    // Constraints
    glm::bvec3 freeze_position{ false };
    glm::bvec3 freeze_rotation{ false };

    void set_mass(float new_mass);

    void set_inertia_tensor(const glm::mat3& new_inertia);

    /**
     * @brief Update physics parameters (call when mass/inertia changes)
     */
    void update_inertia();

    /**
     * @brief Integrate motion over time using Verlet integration
     * @param dt Delta time in seconds
     * @param gravity Global gravity vector
     */
    void integrate(float dt, const glm::vec3 &gravity);

    // Force application methods
    void add_force(const glm::vec3 &force);

    void add_force_at_position(const glm::vec3 &force, const glm::vec3 &position);

    void add_torque(const glm::vec3 &torque);
};
