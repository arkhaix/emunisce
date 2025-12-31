# Platform - header-only library for platform-specific definitions
add_library(platform INTERFACE)
target_include_directories(platform INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/emunisce/platform>
)

# Provide namespaced alias for use via FetchContent/add_subdirectory
add_library(emunisce::platform ALIAS platform)
