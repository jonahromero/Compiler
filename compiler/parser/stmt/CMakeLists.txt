
add_library(stmt INTERFACE)

target_link_libraries(stmt INTERFACE expr lexer)
target_include_directories(stmt INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")