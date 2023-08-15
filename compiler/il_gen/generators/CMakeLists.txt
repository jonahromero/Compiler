
add_subdirectory(expr_generator)

add_library(il_gen_generators STATIC 
	FunctionGenerator.cpp
	ILGenerator.cpp
)

target_include_directories(il_gen_generators 
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
	PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(il_gen_generators PUBLIC il expr_generator il_gen_enviroment)