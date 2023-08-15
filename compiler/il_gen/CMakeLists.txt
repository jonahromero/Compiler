
add_subdirectory(il)

add_library(il_gen STATIC 
	ILGenerator.cpp
	TypeSystem.cpp
)

target_link_libraries(il_gen PUBLIC il PRIVATE lexer parser)
target_include_directories(il_gen PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(il_gen PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")