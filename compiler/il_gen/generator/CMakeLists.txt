
add_library(generator STATIC Generator.cpp)

target_link_libraries(generator PUBLIC il il_gen_enviroment)
target_include_directories(generator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")