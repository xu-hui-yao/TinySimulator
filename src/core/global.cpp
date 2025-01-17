#include <core/global.h>
#include <spdlog/sinks/basic_file_sink.h>

filesystem::resolver &get_file_resolver() {
    static filesystem::resolver resolver;
    return resolver;
}

std::shared_ptr<spdlog::logger> get_logger() {
    static std::shared_ptr<spdlog::logger> logger = [] {
        auto log = spdlog::basic_logger_mt("spdlog", "spdlog.log");
        log->set_pattern("[%n][%Y-%m-%d %H:%M:%S.%e] [%l] [%t]  %v");
        log->set_level(spdlog::level::debug);
        spdlog::flush_every(std::chrono::seconds(10));
        return log;
    }();
    return logger;
}

std::pair<uint32_t, uint32_t> &get_resolution() {
    static std::pair<uint32_t, uint32_t> resolution;
    return resolution;
}

GLFWwindow *get_window() {
    static GLFWwindow *window = nullptr;
    return window;
}

bool *get_keys() {
    static bool keys[1024] = { false };
    return keys;
}

std::tuple<float, float, float> &get_mouse_offset() {
    static std::tuple<float, float, float> offsets;
    return offsets;
}

float &get_delta_time() {
    static float delta_time = 0.0f;
    return delta_time;
}

float &get_last_frame_time() {
    static float last_frame_time = 0.0f;
    return last_frame_time;
}
