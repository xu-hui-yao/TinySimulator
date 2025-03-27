#pragma once

#include <memory>
#include <scene/model/model.h>

struct Renderable {
    enum RenderMode { fill, polygon };

    std::shared_ptr<Model> model;
    RenderMode mode = fill;
};