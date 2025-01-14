#include <assets/texture/texture.h>
#include <core/common.h>
#include <glad.h>

Texture::Texture(int height, int width, int channel) noexcept
    : Resource(filesystem::path()), m_id(0), m_type(ENone), m_channel(channel), m_width(width), m_height(height) {
    m_data = std::make_unique<float[]>(width * height * channel);
}

Texture::Texture() noexcept
    : Resource(filesystem::path()), m_id(0), m_type(ENone), m_channel(0), m_width(0), m_height(0) {}

Texture::~Texture() noexcept {
    if (m_id != 0) {
        glDeleteTextures(1, &m_id);
    }
    m_id = 0;
}

Texture::Texture(Texture &&other) noexcept
    : Resource(other.m_path), m_id(other.m_id), m_type(other.m_type), m_data(std::move(other.m_data)),
      m_channel(other.m_channel), m_width(other.m_width), m_height(other.m_height) {
    other.m_id = 0;
}

Texture &Texture::operator=(Texture &&other) noexcept {
    if (this != &other) {
        // Delete our existing GPU texture (if any)
        unload();

        // Now take the other's data
        m_id      = other.m_id;
        m_type    = other.m_type;
        m_data    = std::move(other.m_data);
        m_channel = other.m_channel;
        m_width   = other.m_width;
        m_height  = other.m_height;

        // Ensure other is nullified
        other.m_id      = 0;
        other.m_width   = 0;
        other.m_height  = 0;
        other.m_channel = 0;
    }
    return *this;
}

void Texture::upload() noexcept {
    if (m_id != 0) {
        return;
    }

    GLint format;
    if (m_channel == 1)
        format = GL_RED;
    else if (m_channel == 3)
        format = GL_RGB;
    else if (m_channel == 4)
        format = GL_RGBA;
    else
        throw std::runtime_error("Unsupported m_channel count: " + std::to_string(m_channel));
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_FLOAT, m_data.get());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::unload() noexcept {
    if (m_id != 0) {
        glDeleteTextures(1, &m_id);
    }
    m_id = 0;
}
