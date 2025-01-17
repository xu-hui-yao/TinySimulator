#pragma once

#include <GLFW/glfw3.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double x_pozs_in, double y_pos_in);

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
