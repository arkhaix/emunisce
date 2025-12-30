cmake_minimum_required(VERSION 3.30)

set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for macOS" FORCE)
set(CMAKE_OSX_DEPLOYMENT_TARGET "15.5" CACHE STRING "" FORCE)
set(VCPKG_TARGET_TRIPLET "arm64-osx" CACHE STRING "VCPKG Target Triplet" FORCE)
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

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

# SDL3 application
option(BUILD_SDL_APPLICATION "Build SDL3 application" ON)

if(BUILD_SDL_APPLICATION)
  add_subdirectory(sdl_application)
endif()

# Windows-specific application
if(WIN32)
  add_subdirectory(windows_application)
endif()
