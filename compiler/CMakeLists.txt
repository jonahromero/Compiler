include(FetchContent)
# Include spdlog for logging
FetchContent_Declare(
    spdlog 
    GIT_REPOSITORY    https://github.com/gabime/spdlog.git
    GIT_TAG	      v1.x
)
set(SPDLOG_SOURCE_DIR "" CACHE FILEPATH "Directory of spdlog source if it exists.")
file(TO_CMAKE_PATH "${SPDLOG_SOURCE_DIR}" SPDLOG_SOURCE_DIR_NORM)

if(SPDLOG_SOURCE_DIR_NORM STREQUAL "")
    message(STATUS "Downloading spdlog")
    FetchContent_MakeAvailable(spdlog)
else()
    message(STATUS "Using user provided spdlog")
    add_subdirectory(SPDLOG_SOURCE_DIR_NORM)
endif()

#find_package(spdlog REQUIRED)

add_compile_options(-Wno-reorder-ctor -Wno-overloaded-virtual -Wno-parentheses-equality)
#Miscellaneous helpers
add_subdirectory(errors)
add_subdirectory(util)

# First stage to create AST
add_subdirectory(lexer)
add_subdirectory(parser)

# Second stage for codegen from AST
add_subdirectory(il_gen)
#add_subdirectory(assembler)

add_library(compiler INTERFACE)
target_link_libraries(compiler INTERFACE lexer parser il_gen)