

add_library(il_gen_generators STATIC 
	ExprGenerator.cpp
	FunctionGenerator.cpp
	ILGenerator.cpp
 "CtrlFlowGraphFlattener.h" "CtrlFlowGraphFlattener.cpp" "ILExprResult.h") 

target_include_directories(il_gen_generators 
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
	PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(il_gen_generators PUBLIC il il_gen_enviroment)