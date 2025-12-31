cmake_minimum_required(VERSION 3.30)

if(APPLE)
  set(VCPKG_TARGET_TRIPLET "arm64-osx" CACHE STRING "VCPKG Target Triplet" FORCE)
  set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for macOS" FORCE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "15.5" CACHE STRING "" FORCE)
elseif(WIN32)
  set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "VCPKG Target Triplet" FORCE)
else()
  set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "VCPKG Target Triplet" FORCE)
endif()

message(STATUS "Using VCPKG target triplet: ${VCPKG_TARGET_TRIPLET}")

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
message(STATUS "Using VCPKG toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

project(emunisce CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_OPENGL_RENDERER "Build OpenGL renderer" ON)
option(BUILD_SDL_GPU_RENDERER "Build SDL GPU renderer" ON)

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
