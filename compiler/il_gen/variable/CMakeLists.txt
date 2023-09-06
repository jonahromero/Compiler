
add_library(il_gen_variable INTERFACE)

target_link_libraries(il_gen_variable INTERFACE il_gen_type_system il)
target_include_directories(il_gen_variable INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
