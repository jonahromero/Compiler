
add_library(default_generator STATIC Generator.cpp GeneratorErrors.cpp "GeneratorToolKit.h" "GeneratorToolKit.cpp" "SimpleGenerator.h" "SimpleGenerator.cpp")

target_link_libraries(default_generator PUBLIC il il_gen_enviroment)
target_include_directories(default_generator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")