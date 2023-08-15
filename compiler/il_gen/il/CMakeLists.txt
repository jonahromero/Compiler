
add_library(il INTERFACE)

target_link_directories(il INTERFACE expr stmt)
target_include_directories(il INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")