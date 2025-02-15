#pragma once

#include <memory>
#include <scene/texture/texture.h>

struct TextureComponent {
    std::shared_ptr<Texture> texture;
};