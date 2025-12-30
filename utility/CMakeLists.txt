# Utility library
add_subdirectory(serialization)
add_subdirectory(hqx)

# Aggregate utility library
add_library(utility INTERFACE)
target_link_libraries(utility INTERFACE
    serialization
    hqx
)
target_include_directories(utility INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
