#include <core/callback.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double x_pos_in, double y_pos_in) {
    static GLfloat lastX = static_cast<GLfloat>(screenWidth) / 2.0f;
    static GLfloat lastY = static_cast<GLfloat>(screenHeight) / 2.0f;
    auto xPos = static_cast<GLfloat>(x_pos_in);
    auto yPos = static_cast<GLfloat>(y_pos_in);
    mouseXOffset = xPos - lastX;
    mouseYOffset = lastY - yPos;
    lastX = xPos;
    lastY = yPos;
}

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset) {
    mouseScrollOffset = static_cast<GLfloat>(y_offset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            keys[key] = GL_TRUE;
        } else if (action == GLFW_RELEASE) {
            keys[key] = GL_FALSE;
        }
    }
}