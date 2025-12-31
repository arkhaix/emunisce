# Win32 common library
add_library(win32_common
    window.cpp
    window.h
)
target_include_directories(win32_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
