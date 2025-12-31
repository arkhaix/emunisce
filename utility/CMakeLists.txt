# Utility library
add_subdirectory(serialization)
add_subdirectory(hqx)

# Aggregate utility library
add_library(utility INTERFACE)
target_link_libraries(utility INTERFACE
    serialization
    hqx
)
target_include_directories(utility INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/emunisce/utility>
)

# Provide namespaced alias for use via FetchContent/add_subdirectory
add_library(emunisce::utility ALIAS utility)
