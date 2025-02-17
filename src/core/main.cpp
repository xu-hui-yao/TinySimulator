#include <core/fwd.h>

int main() {
    // Create window and start loop
    get_window_manager()->init("test", 1920, 1080);

    while (!get_quit()) {
        auto start_time = std::chrono::high_resolution_clock::now();
        get_window_manager()->process_events();
        get_window_manager()->update();
        auto stop_time   = std::chrono::high_resolution_clock::now();
        get_delta_time() = std::chrono::duration<float>(stop_time - start_time).count();
    }

    // Shutdown
    get_window_manager()->shutdown();
    return 0;
}