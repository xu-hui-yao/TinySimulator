#pragma once

#include <assets/texture/texture.h>
#include <memory>

struct TextureComponent {
    std::shared_ptr<Texture> texture;
};