
add_library(stmt INTERFACE)

target_link_libraries(stmt INTERFACE expr)
target_include_directories(stmt INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")