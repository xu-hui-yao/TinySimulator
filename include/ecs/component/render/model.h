#pragma once

#include <memory>
#include <scene/model/model.h>

struct ModelComponent {
    std::shared_ptr<Model> model;
};