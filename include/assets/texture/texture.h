#pragma once

#include <glad.h>
#include <memory>
#include <string>

enum TextureType { EDiffuseType, ESpecularType, ENormalType, EHeightType, ESkyBoxEquivalentRectangularType };

class Texture {
public:
    Texture(const std::string &filename, TextureType texture_type);

    ~Texture();

    Texture(Texture &&other) noexcept;

    Texture &operator=(Texture &&other) noexcept;

    void upload_to_gpu();

    void delete_from_gpu();

private:
    GLuint id;
    TextureType type;
    std::unique_ptr<float[]> data;
    int channel;
    int width;
    int height;
};