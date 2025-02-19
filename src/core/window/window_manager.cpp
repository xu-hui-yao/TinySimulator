#include <glad.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <core/event/event_manager.h>
#include <core/fwd.h>
#include <core/window/window_manager.h>
#include <imgui.h>

void WindowManager::init(const std::string &title, int width, int height) {
    // Init GLFW
    if (!glfwInit()) {
        get_logger()->error("Failed to initialize GLFW");
        return;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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

    // Disable cursor movement
    glfwSetInputMode(m_window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(m_window.get(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

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
    Event event(Events::Window::INPUT);
    glfwPollEvents();

    // Keyboard process
    m_keyboard_state[static_cast<size_t>(InputButtons::W)]     = glfwGetKey(m_window.get(), GLFW_KEY_W);
    m_keyboard_state[static_cast<size_t>(InputButtons::A)]     = glfwGetKey(m_window.get(), GLFW_KEY_A);
    m_keyboard_state[static_cast<size_t>(InputButtons::S)]     = glfwGetKey(m_window.get(), GLFW_KEY_S);
    m_keyboard_state[static_cast<size_t>(InputButtons::D)]     = glfwGetKey(m_window.get(), GLFW_KEY_D);
    m_keyboard_state[static_cast<size_t>(InputButtons::Q)]     = glfwGetKey(m_window.get(), GLFW_KEY_Q);
    m_keyboard_state[static_cast<size_t>(InputButtons::E)]     = glfwGetKey(m_window.get(), GLFW_KEY_E);
    m_keyboard_state[static_cast<size_t>(InputButtons::R)]     = glfwGetKey(m_window.get(), GLFW_KEY_R);
    m_keyboard_state[static_cast<size_t>(InputButtons::SPACE)] = glfwGetKey(m_window.get(), GLFW_KEY_SPACE);
    m_keyboard_state[static_cast<size_t>(InputButtons::SHIFT)] = glfwGetKey(m_window.get(), GLFW_KEY_LEFT_SHIFT);
    m_keyboard_state[static_cast<size_t>(InputButtons::CTRL)]  = glfwGetKey(m_window.get(), GLFW_KEY_LEFT_CONTROL);
    m_keyboard_state[static_cast<size_t>(InputButtons::ESC)]   = glfwGetKey(m_window.get(), GLFW_KEY_ESCAPE);
    event.set_param(Events::Window::Input::KEYBOARD_INPUT, m_keyboard_state);

    // Mouse process
    m_mouse_state[static_cast<size_t>(InputMouses::LEFT_PRESS)] =
        glfwGetMouseButton(m_window.get(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    m_mouse_state[static_cast<size_t>(InputMouses::RIGHT_PRESS)] =
        glfwGetMouseButton(m_window.get(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    m_mouse_state[static_cast<size_t>(InputMouses::LEFT_RELEASE)] =
        glfwGetMouseButton(m_window.get(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE;
    m_mouse_state[static_cast<size_t>(InputMouses::RIGHT_RELEASE)] =
        glfwGetMouseButton(m_window.get(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE;

    event.set_param(Events::Window::Input::MOUSE_INPUT, m_mouse_state);

    double mouse_x, mouse_y;
    glfwGetCursorPos(m_window.get(), &mouse_x, &mouse_y);
    m_mouse_pos.x = static_cast<float>(mouse_x);
    m_mouse_pos.y = static_cast<float>(mouse_y);

    event.set_param(Events::Window::Input::MOUSE_POSITION, m_mouse_pos - m_last_mouse_pos);
    m_last_mouse_pos = m_mouse_pos;

    get_event_manager()->send_event(event);

    // Close window events
    if (glfwWindowShouldClose(m_window.get()) || m_keyboard_state[static_cast<size_t>(InputButtons::ESC)]) {
        get_quit() = true;
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
