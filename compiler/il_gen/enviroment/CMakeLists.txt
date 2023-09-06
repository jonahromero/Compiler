
add_library(il_gen_enviroment STATIC
	Enviroment.cpp
)

target_link_libraries(il_gen_enviroment PUBLIC il il_gen_variable il_gen_errors il_gen_type_system)
target_include_directories(il_gen_enviroment PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(il_gen_enviroment PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")