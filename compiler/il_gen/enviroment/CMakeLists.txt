
add_library(il_gen_enviroment STATIC
	Enviroment.cpp
	ExprInterpreter.cpp
	TypeSystem.cpp
	ComputedExpr.cpp
)

target_link_libraries(il_gen_enviroment PUBLIC il generator)
target_include_directories(il_gen_enviroment PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(il_gen_enviroment PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")