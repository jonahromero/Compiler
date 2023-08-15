
add_library(expr_generator STATIC
	ExprGenerator.cpp
)

target_link_libraries(expr_generator PUBLIC il il_gen_enviroment)
target_include_directories(expr_generator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
