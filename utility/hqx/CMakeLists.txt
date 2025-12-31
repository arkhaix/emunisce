# HQX library
add_library(hqx
    hq2x.cpp
    hq3x.cpp
    hq4x.cpp
    hqx.cpp
    hqx.h
)
target_include_directories(hqx PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/emunisce/utility/hqx>
)
target_link_libraries(hqx PUBLIC
    platform
    machine_interfaces
)

# Provide namespaced alias for use via FetchContent/add_subdirectory
add_library(emunisce::hqx ALIAS hqx)
