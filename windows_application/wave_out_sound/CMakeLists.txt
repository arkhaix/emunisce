# Wave out sound library
add_library(wave_out_sound
    wave_out_sound.cpp
    wave_out_sound.h
)
target_include_directories(wave_out_sound PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(wave_out_sound PUBLIC
    platform
    machine
    base_application_lib
    win32_common
    gdi_plus_renderer_headers
    winmm
)

# Header-only library for cross-dependencies
add_library(wave_out_sound_headers INTERFACE)
target_sources(wave_out_sound_headers INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/wave_out_sound.h
)
target_include_directories(wave_out_sound_headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
