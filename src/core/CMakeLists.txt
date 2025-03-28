add_subdirectory(event)
add_subdirectory(window)
add_subdirectory(filesystem)

target_sources(tiny-simulator PRIVATE
        main.cpp
        fwd.cpp
        common.cpp
)
