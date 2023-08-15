
add_library(il INTERFACE)

target_link_libraries(il INTERFACE parser il_gen_common)
target_include_directories(il INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")