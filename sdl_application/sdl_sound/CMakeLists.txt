# SDL sound library
add_library(sdl_sound
    sdl_sound.cpp
    sdl_sound.h
)
target_include_directories(sdl_sound PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../sdl_gpu_renderer
)
target_link_libraries(sdl_sound PUBLIC
    platform
    machine
    base_application_lib
    SDL3::SDL3-static
)
