#include <core/fwd.h>
#include <ecs/fwd.h>
#include <gui/resource_panel.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

int main() {
    // Create window and start loop
    get_window_manager()->init("test", 1920, 1080);
    get_coordinator()->add_event_listener(Events::Window::QUIT, [](Event &) { get_quit() = true; });

    ResourcePanel resource_panel(M_PROJECT_SOURCE_DIR);

    while (!get_quit()) {
        auto start_time = std::chrono::high_resolution_clock::now();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        get_window_manager()->process_events();
        resource_panel.on_ui_render();
        get_coordinator()->update_system(get_delta_time());

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        get_window_manager()->update();

        auto stop_time   = std::chrono::high_resolution_clock::now();
        get_delta_time() = std::chrono::duration<float>(stop_time - start_time).count();
    }

    // Shutdown
    get_window_manager()->shutdown();
    return 0;
}