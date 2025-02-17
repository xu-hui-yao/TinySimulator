#pragma once
#include <glad.h>
#include <vector>

class FrameBuffer {
public:
    FrameBuffer(GLsizei width, GLsizei height, const std::vector<GLenum> &formats);

    ~FrameBuffer();

    void bind() const;

    void unbind() const;

    [[nodiscard]] GLsizei get_width() const;

    [[nodiscard]] GLsizei get_height() const;

private:
    void check_status();

    GLuint m_fbo = 0;
    GLuint m_rbo = 0;
    std::vector<GLuint> m_color_attachments;
    GLsizei m_width  = 0;
    GLsizei m_height = 0;
    bool m_is_valid   = false;
};