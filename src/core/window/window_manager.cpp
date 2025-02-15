#include <glad.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <core/event/event_manager.h>
#include <core/fwd.h>
#include <core/window/window_manager.h>
#include <ecs/fwd.h>
#include <imgui.h>

void WindowManager::init(const std::string &title, int width, int height) {
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create GLFW window
    m_window = std::shared_ptr<GLFWwindow>(glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr),
                                           glfwDestroyWindow);
    if (m_window == nullptr) {
        get_logger()->error("Failed to create GLFW window");
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(m_window.get());
    glfwSwapInterval(1);

    // load opengl function pointer
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        get_logger()->error("Failed to initialize GLAD");
        glfwTerminate();
        exit(1);
    }

    // OpenGL configuration
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Initialize Imgui
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window.get(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void WindowManager::update() const { glfwSwapBuffers(m_window.get()); }

void WindowManager::process_events() {
    glfwPollEvents();

    // Keyboard process
    std::bitset<10> new_keyboard;
    new_keyboard[static_cast<size_t>(InputButtons::W)]     = glfwGetKey(m_window.get(), GLFW_KEY_W);
    new_keyboard[static_cast<size_t>(InputButtons::A)]     = glfwGetKey(m_window.get(), GLFW_KEY_A);
    new_keyboard[static_cast<size_t>(InputButtons::S)]     = glfwGetKey(m_window.get(), GLFW_KEY_S);
    new_keyboard[static_cast<size_t>(InputButtons::D)]     = glfwGetKey(m_window.get(), GLFW_KEY_D);
    new_keyboard[static_cast<size_t>(InputButtons::Q)]     = glfwGetKey(m_window.get(), GLFW_KEY_Q);
    new_keyboard[static_cast<size_t>(InputButtons::E)]     = glfwGetKey(m_window.get(), GLFW_KEY_E);
    new_keyboard[static_cast<size_t>(InputButtons::R)]     = glfwGetKey(m_window.get(), GLFW_KEY_R);
    new_keyboard[static_cast<size_t>(InputButtons::SPACE)] = glfwGetKey(m_window.get(), GLFW_KEY_SPACE);
    new_keyboard[static_cast<size_t>(InputButtons::SHIFT)] = glfwGetKey(m_window.get(), GLFW_KEY_LEFT_SHIFT);
    new_keyboard[static_cast<size_t>(InputButtons::CTRL)]  = glfwGetKey(m_window.get(), GLFW_KEY_LEFT_CONTROL);

    if (new_keyboard != m_keyboard_state) {
        m_keyboard_state = new_keyboard;
        Event keyboard_event(Events::Window::INPUT);
        keyboard_event.set_param(Events::Window::Input::KEYBOARD_INPUT, m_keyboard_state);
        get_coordinator()->send_event(keyboard_event);
    }

    // Mouse process
    std::bitset<2> new_mouse;
    new_mouse[static_cast<size_t>(InputMouses::LEFT)]  = glfwGetMouseButton(m_window.get(), GLFW_MOUSE_BUTTON_LEFT);
    new_mouse[static_cast<size_t>(InputMouses::RIGHT)] = glfwGetMouseButton(m_window.get(), GLFW_MOUSE_BUTTON_RIGHT);

    if (new_mouse != m_mouse_state) {
        m_mouse_state = new_mouse;
        Event mouse_event(Events::Window::INPUT);
        mouse_event.set_param(Events::Window::Input::MOUSE_INPUT, m_mouse_state);
        get_coordinator()->send_event(mouse_event);
    }

    glfwGetCursorPos(m_window.get(), &m_mouse_pos.x, &m_mouse_pos.y);

    if (m_mouse_pos != m_last_mouse_pos) {
        Event mouse_event(Events::Window::INPUT);
        mouse_event.set_param(Events::Window::Input::MOUSE_POSITION, m_mouse_pos - m_last_mouse_pos);
        get_coordinator()->send_event(mouse_event);
        m_last_mouse_pos = m_mouse_pos;
    }

    // Close window events
    if (glfwWindowShouldClose(m_window.get())) {
        get_coordinator()->send_event(Events::Window::QUIT);
    }
}

void WindowManager::shutdown() {
    // Imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // GLFW
    glfwDestroyWindow(m_window.get());
    glfwTerminate();
    m_window = nullptr;
}

std::shared_ptr<GLFWwindow> WindowManager::get_window() const { return m_window; }
