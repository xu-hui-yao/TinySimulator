#include <glad.h>

#include <core/fwd.h>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <scene/shader/shader.h>
#include <sstream>

Shader::Shader(std::filesystem::path path) noexcept : Resource(std::move(path)) {
    m_vertex_path    = m_path;
    m_fragment_path  = m_path.replace_extension("frag");
    m_geometry_path  = m_path.replace_extension("gs");
    m_tess_ctrl_path = m_path.replace_extension("tc");
    m_tess_eval_path = m_path.replace_extension("te");
}

Shader::~Shader() noexcept {
    if (m_shader_id != 0) {
        glDeleteProgram(m_shader_id);
        m_shader_id = 0;
    }
}

Shader::Shader(Shader &&other) noexcept
    : Resource(std::move(other.m_path)), m_shader_id(other.m_shader_id), m_vertex_path(std::move(other.m_vertex_path)),
      m_fragment_path(std::move(other.m_fragment_path)), m_geometry_path(std::move(other.m_geometry_path)),
      m_tess_ctrl_path(std::move(other.m_tess_ctrl_path)), m_tess_eval_path(std::move(other.m_tess_eval_path)) {
    other.m_shader_id = 0;
}

Shader &Shader::operator=(Shader &&other) noexcept {
    if (this != &other) {
        unload();

        m_path           = std::move(other.m_path);
        m_shader_id      = other.m_shader_id;
        m_vertex_path    = std::move(other.m_vertex_path);
        m_fragment_path  = std::move(other.m_fragment_path);
        m_geometry_path  = std::move(other.m_geometry_path);
        m_tess_ctrl_path = std::move(other.m_tess_ctrl_path);
        m_tess_eval_path = std::move(other.m_tess_eval_path);

        other.m_shader_id = 0;
    }
    return *this;
}

void Shader::upload(std::shared_ptr<ResourceDescriptor> /*resource_descriptor*/) noexcept {
    if (m_shader_id != 0) {
        return;
    }

    std::string vertex_source;
    std::string fragment_source;
    std::string geometry_source;
    std::string tess_ctrl_source;
    std::string tess_eval_source;

    try {
        std::ifstream file(absolute(m_vertex_path).string());
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        vertex_source = stream.str();
    } catch (...) {
        get_logger()->error("[Shader] Failed to read file: " + absolute(m_vertex_path).string());
        return;
    }

    try {
        std::ifstream file(absolute(m_fragment_path).string());
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        fragment_source = stream.str();
    } catch (...) {
        get_logger()->error("[Shader] Failed to read file: " + absolute(m_fragment_path).string());
        return;
    }

    if (exists(m_geometry_path)) {
        try {
            std::ifstream file(absolute(m_geometry_path).string());
            std::stringstream stream;
            stream << file.rdbuf();
            file.close();
            geometry_source = stream.str();
        } catch (...) {
            get_logger()->error("[Shader] Failed to read file: " + absolute(m_geometry_path).string());
            return;
        }
    } else {
        get_logger()->info("[Shader] Geometry shader file not provided, skipping.");
    }

    if (exists(m_tess_ctrl_path)) {
        try {
            std::ifstream file(absolute(m_tess_ctrl_path).string());
            std::stringstream stream;
            stream << file.rdbuf();
            file.close();
            tess_ctrl_source = stream.str();
        } catch (...) {
            get_logger()->error("[Shader] Failed to read file: " + absolute(m_tess_ctrl_path).string());
            return;
        }
    } else {
        get_logger()->info("[Shader] Tessellation control shader file not provided, skipping.");
    }

    if (exists(m_tess_eval_path)) {
        try {
            std::ifstream file(absolute(m_tess_eval_path).string());
            std::stringstream stream;
            stream << file.rdbuf();
            file.close();
            tess_eval_source = stream.str();
        } catch (...) {
            get_logger()->error("[Shader] Failed to read file: " + absolute(m_tess_eval_path).string());
            return;
        }
    } else {
        get_logger()->info("[Shader] Tessellation evaluation shader file not provided, skipping.");
    }

    // compile
    if (!compile_shader(vertex_source, fragment_source, geometry_source, tess_ctrl_source, tess_eval_source)) {
        get_logger()->error("[Shader] Failed to compile shader: " + absolute(m_vertex_path.parent_path()).string());
    }
}

void Shader::unload() noexcept {
    if (m_shader_id != 0) {
        glDeleteProgram(m_shader_id);
        m_shader_id = 0;
    }
}

void Shader::use() const noexcept {
    if (m_shader_id != 0) {
        glUseProgram(m_shader_id);
    }
}

void Shader::set_bool(const char *name, bool value) const noexcept {
    glUniform1i(glGetUniformLocation(m_shader_id, name), value);
}

void Shader::set_float(const char *name, float value) const noexcept {
    glUniform1f(glGetUniformLocation(m_shader_id, name), value);
}

void Shader::set_integer(const char *name, int value) const noexcept {
    glUniform1i(glGetUniformLocation(m_shader_id, name), value);
}

void Shader::set_vector2(const char *name, const glm::vec2 &value) const noexcept {
    glUniform2f(glGetUniformLocation(m_shader_id, name), value.x, value.y);
}

void Shader::set_vector3(const char *name, const glm::vec3 &value) const noexcept {
    glUniform3f(glGetUniformLocation(m_shader_id, name), value.x, value.y, value.z);
}

void Shader::set_vector4(const char *name, const glm::vec4 &value) const noexcept {
    glUniform4f(glGetUniformLocation(m_shader_id, name), value.x, value.y, value.z, value.w);
}

void Shader::set_matrix2(const char *name, const glm::mat2 &matrix) const noexcept {
    glUniformMatrix2fv(glGetUniformLocation(m_shader_id, name), 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::set_matrix3(const char *name, const glm::mat3 &matrix) const noexcept {
    glUniformMatrix3fv(glGetUniformLocation(m_shader_id, name), 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::set_matrix4(const char *name, const glm::mat4 &matrix) const noexcept {
    glUniformMatrix4fv(glGetUniformLocation(m_shader_id, name), 1, GL_FALSE, glm::value_ptr(matrix));
}

bool Shader::compile_shader(const std::string &vertex_source, const std::string &fragment_source,
                            const std::string &geometry_source, const std::string &tess_ctrl_source,
                            const std::string &tess_eval_source) {
    unsigned int vertex = 0, fragment = 0;
    unsigned int geometry = 0, tess_ctrl = 0, tess_eval = 0;

    // vertex
    vertex            = glCreateShader(GL_VERTEX_SHADER);
    const char *v_src = vertex_source.c_str();
    glShaderSource(vertex, 1, &v_src, nullptr);
    glCompileShader(vertex);
    if (!check_compile_errors(vertex, ShaderType::EVertex)) {
        glDeleteShader(vertex);
        return false;
    }

    // fragment
    fragment          = glCreateShader(GL_FRAGMENT_SHADER);
    const char *f_src = fragment_source.c_str();
    glShaderSource(fragment, 1, &f_src, nullptr);
    glCompileShader(fragment);
    if (!check_compile_errors(fragment, ShaderType::EFragment)) {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return false;
    }

    // geometry (optional)
    if (!geometry_source.empty()) {
        geometry          = glCreateShader(GL_GEOMETRY_SHADER);
        const char *g_src = geometry_source.c_str();
        glShaderSource(geometry, 1, &g_src, nullptr);
        glCompileShader(geometry);
        if (!check_compile_errors(geometry, ShaderType::EGeometry)) {
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            glDeleteShader(geometry);
            return false;
        }
    }

    // tessellation control (optional)
    if (!tess_ctrl_source.empty()) {
        tess_ctrl          = glCreateShader(GL_TESS_CONTROL_SHADER);
        const char *tc_src = tess_ctrl_source.c_str();
        glShaderSource(tess_ctrl, 1, &tc_src, nullptr);
        glCompileShader(tess_ctrl);
        if (!check_compile_errors(tess_ctrl, ShaderType::ETessellationControl)) {
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            if (geometry)
                glDeleteShader(geometry);
            glDeleteShader(tess_ctrl);
            return false;
        }
    }

    // tessellation eval (optional)
    if (!tess_eval_source.empty()) {
        tess_eval          = glCreateShader(GL_TESS_EVALUATION_SHADER);
        const char *te_src = tess_eval_source.c_str();
        glShaderSource(tess_eval, 1, &te_src, nullptr);
        glCompileShader(tess_eval);
        if (!check_compile_errors(tess_eval, ShaderType::ETessellationEvaluation)) {
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            if (geometry)
                glDeleteShader(geometry);
            if (tess_ctrl)
                glDeleteShader(tess_ctrl);
            glDeleteShader(tess_eval);
            return false;
        }
    }

    // create program
    m_shader_id = glCreateProgram();
    glAttachShader(m_shader_id, vertex);
    glAttachShader(m_shader_id, fragment);
    if (geometry)
        glAttachShader(m_shader_id, geometry);
    if (tess_ctrl)
        glAttachShader(m_shader_id, tess_ctrl);
    if (tess_eval)
        glAttachShader(m_shader_id, tess_eval);

    glLinkProgram(m_shader_id);
    if (!check_compile_errors(m_shader_id, ShaderType::EProgram)) {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (geometry)
            glDeleteShader(geometry);
        if (tess_ctrl)
            glDeleteShader(tess_ctrl);
        if (tess_eval)
            glDeleteShader(tess_eval);
        glDeleteProgram(m_shader_id);
        m_shader_id = 0;
        return false;
    }

    // clean up shaders
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometry)
        glDeleteShader(geometry);
    if (tess_ctrl)
        glDeleteShader(tess_ctrl);
    if (tess_eval)
        glDeleteShader(tess_eval);

    return true;
}

bool Shader::check_compile_errors(unsigned int object, const ShaderType &type) {
    int success;
    char info_log[1024];

    if (type == ShaderType::EProgram) {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(object, 1024, nullptr, info_log);
            get_logger()->error("[Shader] Program link error: " + std::string(info_log));
            return false;
        }
    } else {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(object, 1024, nullptr, info_log);
            get_logger()->error("[Shader] compile error: " + std::string(info_log));
            return false;
        }
    }
    return true;
}
