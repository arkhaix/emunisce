if(NOT BUILD_SDL_GPU_RENDERER)
    message(STATUS "BUILD_SDL_GPU_RENDERER is OFF, skipping sdl_gpu_renderer")
    return()
endif()

find_package(SDL3 REQUIRED)

add_library(sdl_gpu_renderer
    sdl_gpu_renderer.cpp
    sdl_gpu_renderer.h
)
target_include_directories(sdl_gpu_renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sdl_gpu_renderer PUBLIC
    platform
    machine
    SDL3::SDL3
)
