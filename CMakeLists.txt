cmake_minimum_required(VERSION 3.19)
project(tiny-renderer LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(tiny-renderer)

add_subdirectory(ext)
add_subdirectory(src)

target_include_directories(tiny-renderer PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(tiny-renderer PRIVATE
        Threads::Threads
        OpenGL::GL
)

target_compile_definitions(tiny-renderer PRIVATE
        $<$<CONFIG:Debug>:M_DEBUG>
        M_PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
        GL_SILENCE_DEPRECATION
)

if (MSVC)
    add_compile_options("/utf-8")
    target_compile_options(tiny-renderer PRIVATE "/utf-8")
endif()
