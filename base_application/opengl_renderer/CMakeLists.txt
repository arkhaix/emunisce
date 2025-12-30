if(NOT BUILD_OPENGL_RENDERER)
    message(STATUS "BUILD_OPENGL_RENDERER is OFF, skipping opengl_renderer")
    return()
endif()

find_package(OpenGL REQUIRED)

add_library(opengl_renderer
    opengl_renderer.cpp
    glext.h
    opengl_renderer.h
)
target_include_directories(opengl_renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(opengl_renderer PUBLIC
    platform
    machine
    OpenGL::GL
)
