add_subdirectory(event)
add_subdirectory(window)
add_subdirectory(filesystem)

target_sources(tiny-renderer PRIVATE
        main.cpp
        fwd.cpp
        common.cpp
)
