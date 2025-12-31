# Machine library

# Machine interfaces - header-only library
add_library(machine_interfaces INTERFACE)
target_include_directories(machine_interfaces INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(machine_interfaces INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/emulated_display.h
    ${CMAKE_CURRENT_SOURCE_DIR}/emulated_input.h
    ${CMAKE_CURRENT_SOURCE_DIR}/emulated_machine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/emulated_memory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/emulated_processor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/emulated_sound.h
    ${CMAKE_CURRENT_SOURCE_DIR}/machine_factory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/machine_includes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/machine_serialization.h
    ${CMAKE_CURRENT_SOURCE_DIR}/machine_to_application.h
    ${CMAKE_CURRENT_SOURCE_DIR}/machine_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/screen_buffer.h
)

# Machine implementation library
add_library(machine
    machine_factory.cpp
    machine_serialization.cpp
    screen_buffer.cpp
    emulated_display.h
    emulated_input.h
    emulated_machine.h
    emulated_memory.h
    emulated_processor.h
    emulated_sound.h
    machine_factory.h
    machine_includes.h
    machine_serialization.h
    machine_to_application.h
    machine_types.h
    screen_buffer.h
    t_screen_buffer.hpp
)
target_include_directories(machine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(machine PUBLIC
    platform
    utility
    gameboy
)
