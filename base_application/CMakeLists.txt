# Base application library
add_subdirectory(base_application)
add_subdirectory(opengl_renderer)
add_subdirectory(sdl_gpu_renderer)

# Aggregate base_application library
add_library(base_application_lib INTERFACE)
target_link_libraries(base_application_lib INTERFACE
    base_application
)

# Link opengl_renderer if it was built
if(TARGET opengl_renderer)
    target_link_libraries(base_application_lib INTERFACE opengl_renderer)
endif()

# Link sdl_gpu_renderer if it was built
if(TARGET sdl_gpu_renderer)
    target_link_libraries(base_application_lib INTERFACE sdl_gpu_renderer)
endif()
