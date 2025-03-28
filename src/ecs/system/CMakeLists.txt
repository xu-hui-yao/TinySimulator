add_subdirectory(physics_subsystem)

target_sources(tiny-simulator PRIVATE
        render.cpp
        physics.cpp
        input.cpp
)
