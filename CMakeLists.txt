cmake_minimum_required(VERSION 3.30)

# Determine if emunisce is being built as a top-level project or as a subproject
set(EMUNISCE_MAIN_PROJECT OFF)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(EMUNISCE_MAIN_PROJECT ON)
endif()

# Only configure vcpkg and toolchain when building as the main project
if(EMUNISCE_MAIN_PROJECT)
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

  if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "Using VCPKG toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
  endif()
endif()

project(emunisce
  VERSION 1.0.0
  LANGUAGES CXX
  DESCRIPTION "GameBoy emulator library"
)

# C++ standard settings - only set if not already defined by parent project
if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(NOT DEFINED CMAKE_CXX_STANDARD_REQUIRED)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

# Build options - applications default to ON for main project, OFF for subproject
option(EMUNISCE_BUILD_APPLICATIONS "Build emunisce applications" ${EMUNISCE_MAIN_PROJECT})
option(BUILD_OPENGL_RENDERER "Build OpenGL renderer" ${EMUNISCE_MAIN_PROJECT})
option(BUILD_SDL_GPU_RENDERER "Build SDL GPU renderer" ${EMUNISCE_MAIN_PROJECT})
option(BUILD_SDL_APPLICATION "Build SDL3 application" ${EMUNISCE_MAIN_PROJECT})

# Add subdirectories for core libraries (always built)
add_subdirectory(platform)
add_subdirectory(utility)
add_subdirectory(machine)
add_subdirectory(gameboy)

# Applications - only build when requested
if(EMUNISCE_BUILD_APPLICATIONS)
  add_subdirectory(base_application)

  if(BUILD_SDL_APPLICATION)
    add_subdirectory(sdl_application)
  endif()

  # Windows-specific application
  if(WIN32)
    add_subdirectory(windows_application)
  endif()
endif()
