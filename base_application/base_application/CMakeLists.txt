# Base application library
add_library(base_application
    base_application.cpp
    command_trie.cpp
    gui.cpp
    input_manager.cpp
    input_recording.cpp
    kings_dream.cpp
    machine_feature.cpp
    machine_runner.cpp
    rewinder.cpp
    base_application.h
    command_trie.h
    gui.h
    input_manager.h
    input_recording.h
    kings_dream.h
    machine_feature.h
    machine_runner.h
    rewinder.h
    user_interface.h
)
target_include_directories(base_application PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(base_application PUBLIC
    platform
    utility
    machine
)
