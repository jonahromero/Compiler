
add_library(parser STATIC 
    BlockParser.cpp
    ExprParser.cpp
    Parser.cpp
    StmtParser.cpp 
)

add_subdirectory(expr)
add_subdirectory(stmt)

target_link_libraries(parser PUBLIC expr stmt common util lexer)
target_include_directories(parser PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(parser PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")