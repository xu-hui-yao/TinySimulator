#include <assets/texture/texture.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>

Texture::Texture(const std::string &filename, TextureType texture_type)
    : id(0), type(texture_type), channel(0), width(0), height(0) {
    float *raw_data = stbi_loadf(filename.c_str(), &width, &height, &channel, 0);
    if (!raw_data) {
        throw std::runtime_error("Failed to load texture: " + filename);
    }
    data = std::unique_ptr<float[]>(raw_data);
}

Texture::~Texture() {
    delete_from_gpu();
}

Texture::Texture(Texture &&other) noexcept
    : id(other.id), type(other.type), data(std::move(other.data)), channel(other.channel), width(other.width),
      height(other.height) {
    other.id = 0;
}

Texture &Texture::operator=(Texture &&other) noexcept {
    if (this != &other) {
        id       = other.id;
        type     = other.type;
        data     = std::move(other.data);
        channel  = other.channel;
        width    = other.width;
        height   = other.height;
        delete_from_gpu();
    }
    return *this;
}

void Texture::upload_to_gpu() {
    if (id != 0) {
        std::cout << "Texture already generated and bound." << std::endl;
        return;
    }

    GLint format;
    if (channel == 1)
        format = GL_RED;
    else if (channel == 3)
        format = GL_RGB;
    else if (channel == 4)
        format = GL_RGBA;
    else
        throw std::runtime_error("Unsupported channel count: " + std::to_string(channel));
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_FLOAT, data.get());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::delete_from_gpu() {
    if (id != 0) {
        glDeleteTextures(1, &id);
    }
    id = 0;
}

