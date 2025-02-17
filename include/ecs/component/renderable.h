#pragma once

#include <memory>
#include <scene/model/model.h>

struct Renderable {
    std::shared_ptr<Model> model;
};