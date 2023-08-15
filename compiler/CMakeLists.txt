
cmake_minimum_required(VERSION 3.10)
project(compiler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_STANDARD_REQUIRED ON)

# Include spdlog for logging
FetchContent_Declare(
    spdlog_repository 
    GIT_REPOSITORY    https://github.com/gabime/spdlog.git
)
set(SPDLOG_SOURCE_DIR "" CACHE FILEPATH "Directory of spdlog source if it exists.")
file(TO_CMAKE_PATH "${SPDLOG_SOURCE_DIR}" SPDLOG_SOURCE_DIR_NORM)
if(SPDLOG_SOURCE_DIR_NORM STREQUAL "")
    FetchContent_MakeAvailable(spdlog_repository spdlog)
else()
    add_subdirectory(SPDLOG_SOURCE_DIR_NORM)
endif()


# First stage to create AST
add_subdirectory(lexer)
add_subdirectory(parser)

# Second stage for codegen from AST
add_subdirectory(il_gen)
add_subdirectory(assembler)

#Miscellaneous helpers
add_subdirectory(common)
add_subdirectory(util)