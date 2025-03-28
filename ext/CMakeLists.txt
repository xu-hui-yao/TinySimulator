# glfw
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation" FORCE)
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target" FORCE)

add_subdirectory(glfw)

# glad
add_library(glad STATIC
        glad/src/glad.c
)

# imgui
add_library(imgui STATIC
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
        imgui
        imgui/backends
        glfw/include
)

target_include_directories(glad PUBLIC
        glad/include
)

#assimp
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "Disable Assimp unit tests" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE INTERNAL "Disable export formats" FORCE)
add_subdirectory(assimp)

# include
target_include_directories(tiny-simulator PRIVATE
        glfw/include
        tinyexr
        tinygltf
        stb
        spdlog/include
        imgui
        imgui/backends
        glm
        assimp/include
        entt/src
)

target_link_libraries(tiny-simulator PRIVATE
        glad
        glfw
        assimp
        imgui
)
