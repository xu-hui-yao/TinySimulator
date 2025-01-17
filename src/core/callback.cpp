#include <core/callback.h>
#include <core/global.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height) { glViewport(0, 0, width, height); }

void mouse_callback(GLFWwindow *window, double x_pos_in, double y_pos_in) {
    static GLfloat lastX                    = static_cast<GLfloat>(get_resolution().first) / 2.0f;
    static GLfloat lastY                    = static_cast<GLfloat>(get_resolution().second) / 2.0f;
    auto xPos                               = static_cast<GLfloat>(x_pos_in);
    auto yPos                               = static_cast<GLfloat>(y_pos_in);
    std::get<0>(get_mouse_offset()) = xPos - lastX;
    std::get<1>(get_mouse_offset()) = lastY - yPos;
    lastX                                   = xPos;
    lastY                                   = yPos;
}

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset) {
    std::get<2>(get_mouse_offset()) = static_cast<GLfloat>(y_offset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            get_keys()[key] = GL_TRUE;
        } else if (action == GLFW_RELEASE) {
            get_keys()[key] = GL_FALSE;
        }
    }
}