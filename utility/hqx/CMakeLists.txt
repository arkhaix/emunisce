# HQX library
add_library(hqx
    hq2x.cpp
    hq3x.cpp
    hq4x.cpp
    hqx.cpp
    hqx.h
)
target_include_directories(hqx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(hqx PUBLIC
    platform
    machine_interfaces
)
