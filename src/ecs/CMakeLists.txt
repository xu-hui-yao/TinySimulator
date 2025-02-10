add_subdirectory(entity)
add_subdirectory(component)
add_subdirectory(system)

target_sources(tiny-renderer PRIVATE
        coordinator.cpp
        fwd.cpp
)
