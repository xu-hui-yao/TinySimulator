add_subdirectory(event)

target_sources(tiny-renderer PRIVATE
        main.cpp
        loop.cpp
        callback.cpp
        global.cpp
)
