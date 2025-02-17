#include <renderer/frame_buffer.h>
#include <stdexcept>

FrameBuffer::FrameBuffer(GLsizei width, GLsizei height, const std::vector<GLenum>& formats)
    : m_width(width), m_height(height) {
    glGenFramebuffers(1, &m_fbo);
    bind();

    // Color attachments
    for (size_t i = 0; i < formats.size(); ++i) {
        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(formats[i]), width, height, 0,
                    GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                              GL_TEXTURE_2D, tex, 0);
        m_color_attachments.push_back(tex);
    }

    // Depth-stencil buffer
    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                             GL_RENDERBUFFER, m_rbo);

    check_status();
    unbind();
}

FrameBuffer::~FrameBuffer() {

}


void FrameBuffer::bind() const {

}

void FrameBuffer::unbind() const {

}

GLsizei FrameBuffer::get_width() const {
    return m_width;
}

GLsizei FrameBuffer::get_height() const {
    return m_height;
}

void FrameBuffer::check_status() {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    m_is_valid = status == GL_FRAMEBUFFER_COMPLETE;
    if (!m_is_valid) {
        throw std::runtime_error("Framebuffer incomplete: " + std::to_string(status));
    }
}