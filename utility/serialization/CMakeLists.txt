add_library(serialization
    archive.cpp
    file_serializer.cpp
    memory_serializer.cpp
    serialize_item.cpp
    archive.h
    file_serializer.h
    memory_serializer.h
    serialization_includes.h
    serialize_item.h
    serializer.h
)
target_include_directories(serialization PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(serialization PUBLIC platform)
