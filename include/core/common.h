#pragma once

#define M_MSAA_LEVEL 4
#define M_MAX_BONE_INFLUENCE 4
#define M_MAX_TEXTURE_COUNT 1024
#define M_HOT_RELOAD_SECONDS 2
#define M_TEXTURE_LOAD_THREAD 1
#define M_MAX_MODEL_COUNT 512
#define M_MODEL_LOAD_THREAD 1
#define M_MAX_SHADER_COUNT 64
#define M_SHADER_LOAD_THREAD 1

#include <core/filesystem/resolver.h>
#include <glfw/glfw3.h>
#include <memory>
#include <spdlog/spdlog.h>

namespace global {

filesystem::resolver &get_file_resolver();

std::shared_ptr<spdlog::logger> get_logger();

std::pair<uint32_t, uint32_t> &get_resolution();

GLFWwindow *get_window();

bool *get_keys();

std::tuple<float, float, float> &get_mouse_offset();

float &get_delta_time();

float &get_last_frame_time();

} // namespace global