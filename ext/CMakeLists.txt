set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation" FORCE)
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target" FORCE)

add_subdirectory(glfw)

add_library(glad STATIC
        glad/src/glad.c
)

target_include_directories(glad PUBLIC
        glad/include
)

target_include_directories(tiny-renderer PRIVATE
        glad/include
        glfw/include
        tinyexr
        tinygltf
        stb
        spdlog
        imgui
        glm
        assimp
)

target_link_libraries(tiny-renderer PRIVATE
        glad
        glfw
)
