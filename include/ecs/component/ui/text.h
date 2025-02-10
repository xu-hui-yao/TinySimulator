#pragma once

#include <glm/glm.hpp>
#include <string>

struct UITextComponent {
    glm::vec2 position = {0.0f, 0.0f};
    glm::vec2 size = {100.0f, 50.0f};
    std::string text;
};