
add_library(lexer STATIC 
    Lexers.cpp
)

target_link_libraries(lexer PUBLIC errors util)
target_include_directories(lexer PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(lexer PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/include")