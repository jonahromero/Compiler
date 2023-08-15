
add_library(errors INTERFACE)

target_link_libraries(errors INTERFACE spdlog::spdlog)
target_include_directories(errors INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")