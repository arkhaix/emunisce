# Windows application
if(NOT WIN32)
    message(WARNING "windows_application is only for Windows platforms")
    return()
endif()

add_subdirectory(win32_common)
add_subdirectory(gdi_plus_renderer)
add_subdirectory(wave_out_sound)
add_subdirectory(emunisce)
