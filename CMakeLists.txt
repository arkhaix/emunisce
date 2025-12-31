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

# Installation configuration
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install core libraries
install(TARGETS platform machine_interfaces machine gameboy serialization hqx utility
  EXPORT emunisceTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers
install(FILES
  platform/platform_defines.h
  platform/platform_includes.h
  platform/platform_types.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/emunisce/platform
)

install(FILES
  machine/emulated_display.h
  machine/emulated_input.h
  machine/emulated_machine.h
  machine/emulated_memory.h
  machine/emulated_processor.h
  machine/emulated_sound.h
  machine/machine_factory.h
  machine/machine_includes.h
  machine/machine_serialization.h
  machine/machine_to_application.h
  machine/machine_types.h
  machine/screen_buffer.h
  machine/t_screen_buffer.hpp
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/emunisce/machine
)

install(FILES
  gameboy/channel_controller.h
  gameboy/cpu.h
  gameboy/display.h
  gameboy/duty_unit.h
  gameboy/envelope_unit.h
  gameboy/gameboy.h
  gameboy/gameboy_includes.h
  gameboy/gameboy_types.h
  gameboy/input.h
  gameboy/length_unit.h
  gameboy/mbc1.h
  gameboy/mbc3.h
  gameboy/mbc5.h
  gameboy/memory.h
  gameboy/rom_only.h
  gameboy/sound.h
  gameboy/sound1.h
  gameboy/sound2.h
  gameboy/sound3.h
  gameboy/sound4.h
  gameboy/sound_generator.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/emunisce/gameboy
)

install(FILES
  utility/serialization/archive.h
  utility/serialization/file_serializer.h
  utility/serialization/memory_serializer.h
  utility/serialization/serialization_includes.h
  utility/serialization/serialize_item.h
  utility/serialization/serializer.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/emunisce/utility/serialization
)

install(FILES
  utility/hqx/hqx.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/emunisce/utility/hqx
)

# Install SDL application if built
if(EMUNISCE_BUILD_APPLICATIONS AND BUILD_SDL_APPLICATION AND TARGET emunisce_sdl)
  install(TARGETS emunisce_sdl
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
endif()

# Generate and install package configuration files
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/emunisceConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/emunisceConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/emunisceConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/emunisce
)

install(EXPORT emunisceTargets
  FILE emunisceTargets.cmake
  NAMESPACE emunisce::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/emunisce
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/emunisceConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/emunisceConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/emunisce
)
