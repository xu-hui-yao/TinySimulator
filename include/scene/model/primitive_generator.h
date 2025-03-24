#pragma once

#include <any>
#include <scene/model/model.h>
#include <string>

/**
 * @brief Generates primitive 3D models programmatically
 *
 * Supports parameterized generation of basic geometric shapes with configurable
 * attributes and material properties. All generated models follow the resource
 * management system conventions.
 */
class PrimitiveGenerator {
public:
    /**
     * @brief Generates a primitive model of specified type
     *
     * @param type Shape type (cube, sphere, plane, etc.)
     * @param params Generation parameters map with std::any values
     * @return std::shared_ptr<Model> Generated model or nullptr if type invalid
     */
    static std::shared_ptr<Model> generate(const std::string &type,
                                           const std::unordered_map<std::string, std::any> &params);

private:
    // Shape generation implementations
    static std::shared_ptr<Model> generate_cube(const std::unordered_map<std::string, std::any> &params);

    static std::shared_ptr<Model> generate_sphere(const std::unordered_map<std::string, std::any> &params);

    static std::shared_ptr<Model> generate_plane(const std::unordered_map<std::string, std::any> &params);

    /**
     * @brief Calculates tangent vectors for normal mapping
     *
     * @param vertices Vertex array to modify
     * @param indices Index array defining triangle faces
     */
    static void calculate_tangents(std::vector<Vertex> &vertices, const std::vector<GLuint> &indices);
};
