
add_library(il_gen_type_system STATIC
	ComputedExpr.cpp
	ExprInterpreter.cpp 
	TypeSystem.cpp
	TargetInfo.cpp
)

target_link_libraries(il_gen_type_system PUBLIC il util il_gen_common il_gen_errors)
target_include_directories(il_gen_type_system 
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"	
)