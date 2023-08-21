

add_library(il_gen_generators STATIC 
	FunctionGenerator.cpp
	FunctionHelpers.cpp
	ILGenerator.cpp
	ExprGenerator.cpp
 )

target_include_directories(il_gen_generators 
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
	PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(il_gen_generators PUBLIC 
	il 
	generator
	il_gen_enviroment 
	il_gen_ctrl_flow_graph
)