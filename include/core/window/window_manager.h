#pragma once

#if defined(_WIN32)
#undef APIENTRY
#endif
#include <GLFW/glfw3.h>
#include <bitset>
#include <glm/glm.hpp>

class WindowManager {
public:
    void init(const std::string &title, int width, int height);

    void update() const;

    void process_events();

    void shutdown();

    [[nodiscard]] std::shared_ptr<GLFWwindow> get_window() const;

private:
    std::shared_ptr<GLFWwindow> m_window{nullptr};
    std::bitset<11> m_keyboard_state{};
    std::bitset<4> m_mouse_state{};
    glm::vec2 m_mouse_pos{};
    glm::vec2 m_last_mouse_pos{};
    double scroll_x{}, scroll_y{};
};