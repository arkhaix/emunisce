cmake_minimum_required(VERSION 3.15)
project(emunisce CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_OPENGL_RENDERER "Build OpenGL renderer" ON)

# Add subdirectories for core libraries
add_subdirectory(platform)
add_subdirectory(utility)
add_subdirectory(machine)
add_subdirectory(gameboy)
add_subdirectory(base_application)

# Windows-specific application
if(WIN32)
    add_subdirectory(windows_application)
endif()
