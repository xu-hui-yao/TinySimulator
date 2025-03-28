cmake_minimum_required(VERSION 3.19)
project(tiny-simulator LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(tiny-simulator
        src/ecs/component/transform.cpp
        src/ecs/system/render.cpp
        src/ecs/system/physics.cpp
        include/ecs/component/transform.h
        include/ecs/component/rigidbody.h
        include/ecs/component/renderable.h
        include/ecs/system/render.h
        include/ecs/system/physics.h
        include/ecs/component/collider.h)

add_subdirectory(ext)
add_subdirectory(src)

target_include_directories(tiny-simulator PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(tiny-simulator PRIVATE
        Threads::Threads
        OpenGL::GL
)

target_compile_definitions(tiny-simulator PRIVATE
        $<$<CONFIG:Debug>:M_DEBUG>
        M_PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
        GL_SILENCE_DEPRECATION
        GLM_ENABLE_EXPERIMENTAL
)

if (MSVC)
    add_compile_options("/utf-8")
    target_compile_options(tiny-simulator PRIVATE "/utf-8")
endif()
