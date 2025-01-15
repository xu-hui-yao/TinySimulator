add_subdirectory(model)
add_subdirectory(texture)

target_sources(tiny-renderer PRIVATE
    fwd.cpp
)