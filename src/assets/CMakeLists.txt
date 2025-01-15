add_subdirectory(model)
add_subdirectory(texture)
add_subdirectory(shader)

target_sources(tiny-renderer PRIVATE
    fwd.cpp
)