# Emunisce Windows executable
add_executable(emunisce_windows WIN32
    console_debugger.cpp
    emunisce.cpp
    main.cpp
    console_debugger.h
    emunisce.h
)
target_include_directories(emunisce_windows PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(emunisce_windows PUBLIC
    platform
    machine
    base_application_lib
    win32_common
    gdi_plus_renderer
    wave_out_sound
    shlwapi
)
