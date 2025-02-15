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
#include <core/window/window_manager.h>
#if defined(_WIN32)
#undef APIENTRY
#endif
#include <GLFW/glfw3.h>
#include <cstdint>
#include <spdlog/spdlog.h>

Resolver &get_file_resolver();

std::shared_ptr<spdlog::logger> get_logger();

std::shared_ptr<WindowManager> get_window_manager();

bool &get_quit();

std::pair<uint32_t, uint32_t> &get_resolution();

bool *get_keys();

std::tuple<float, float, float> &get_mouse_offset();

float &get_delta_time();

float &get_last_frame_time();
