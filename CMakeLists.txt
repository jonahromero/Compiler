
cmake_minimum_required(VERSION 3.10)
project(compiler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_STANDARD_REQUIRED ON)

add_subdirectory(compiler)
add_subdirectory(command_line)
add_subdirectory(tests)