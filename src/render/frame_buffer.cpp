#include <glad.h>

#include <core/fwd.h>
#include <renderer/frame_buffer.h>

FrameBuffer::FrameBuffer(GLsizei width, GLsizei height, const std::vector<GLenum> &formats)
    : m_width(width), m_height(height) {
    glGenFramebuffers(1, &m_fbo);

    bind();

    // Depth attachment
    glGenTextures(1, &m_depth_attachment);
    glBindTexture(GL_TEXTURE_2D, m_depth_attachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_attachment, 0);

    // Color attachments
    for (size_t i = 0; i < formats.size(); ++i) {
        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(formats[i]), width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tex, 0);
        m_color_attachments.push_back(tex);
    }

    // MRT
    if (formats.empty()) {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    } else {
        auto draw_buffers = new GLenum[formats.size()];
        for (int i = 0; i < formats.size(); i++) {
            draw_buffers[i] = GL_COLOR_ATTACHMENT0 + i;
        }
        glDrawBuffers(static_cast<GLsizei>(formats.size()), draw_buffers);
        delete[] draw_buffers;
    }

    // Depth-stencil buffer
    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

    check_status();

    unbind();
}

FrameBuffer::~FrameBuffer() {
    glDeleteTextures(static_cast<GLsizei>(m_color_attachments.size()), m_color_attachments.data());
    glDeleteRenderbuffers(1, &m_rbo);
    glDeleteFramebuffers(1, &m_fbo);
}

void FrameBuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    m_bind = true;
}

void FrameBuffer::unbind() const {
    if (m_bind) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

GLsizei FrameBuffer::get_width() const { return m_width; }

GLsizei FrameBuffer::get_height() const { return m_height; }

void FrameBuffer::check_status() {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    m_is_valid    = status == GL_FRAMEBUFFER_COMPLETE;
    if (!m_is_valid) {
        get_logger()->error("FrameBuffer::check_status: invalid frame buffer");
    }
}