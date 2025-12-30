# GDI Plus renderer library
add_library(gdi_plus_renderer
    gdi_plus_renderer.cpp
    gdi_plus_renderer.h
)
target_include_directories(gdi_plus_renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gdi_plus_renderer PUBLIC
    platform
    machine
    base_application_lib
    win32_common
    wave_out_sound_headers
    gdiplus
)

# Header-only library for cross-dependencies
add_library(gdi_plus_renderer_headers INTERFACE)
target_sources(gdi_plus_renderer_headers INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/gdi_plus_renderer.h
)
target_include_directories(gdi_plus_renderer_headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
