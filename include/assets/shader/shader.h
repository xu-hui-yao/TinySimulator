#pragma once

#include <assets/resource/resource.h>
#include <glm/glm.hpp>

class Shader : public Resource {
public:
    explicit Shader(filesystem::path path) noexcept;
    ~Shader() noexcept override;

    Shader(const Shader &)            = delete;
    Shader &operator=(const Shader &) = delete;

    Shader(Shader &&other) noexcept;
    Shader &operator=(Shader &&other) noexcept;

    void upload(std::shared_ptr<ResourceDescriptor> resource_descriptor) noexcept override;
    void unload() noexcept override;

    // Shader use function
    void use() const noexcept;

    // Uniform helpers
    void set_bool(const char *name, bool value) const noexcept;
    void set_float(const char *name, float value) const noexcept;
    void set_integer(const char *name, int value) const noexcept;
    void set_vector2f(const char *name, const glm::vec2 &value) const noexcept;
    void set_vector3f(const char *name, const glm::vec3 &value) const noexcept;
    void set_vector4f(const char *name, const glm::vec4 &value) const noexcept;
    void set_matrix2(const char *name, const glm::mat2 &matrix) const noexcept;
    void set_matrix3(const char *name, const glm::mat3 &matrix) const noexcept;
    void set_matrix4(const char *name, const glm::mat4 &matrix) const noexcept;

    [[nodiscard]] unsigned int get_shader_id() const noexcept { return m_shader_id; }

private:
    enum class ShaderType { EVertex, EFragment, EGeometry, ETessellationEvaluation, ETessellationControl, EProgram };

    unsigned int m_shader_id{ 0 };
    filesystem::path m_vertex_path;
    filesystem::path m_fragment_path;
    filesystem::path m_geometry_path;
    filesystem::path m_tess_ctrl_path;
    filesystem::path m_tess_eval_path;

    // Utility
    static bool check_compile_errors(unsigned int object, const ShaderType &type);

    // Actual compile and link from source code
    bool compile_shader(const std::string &vertex_source, const std::string &fragment_source,
                        const std::string &geometry_source, const std::string &tess_ctrl_source,
                        const std::string &tess_eval_source);
};
