#pragma once

#include <assets/model/model.h>
#include <memory>

struct ModelComponent {
    std::shared_ptr<Model> model;
};