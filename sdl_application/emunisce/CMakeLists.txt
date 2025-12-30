# Emunisce SDL3 executable
add_executable(emunisce_sdl
    console_debugger.cpp
    emunisce.cpp
    main.cpp
    console_debugger.h
    emunisce.h
)
target_include_directories(emunisce_sdl PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)
target_link_libraries(emunisce_sdl PUBLIC
    platform
    machine
    base_application_lib
    SDL3::SDL3-static
)
