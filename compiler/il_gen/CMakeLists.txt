
add_subdirectory(common)
add_subdirectory(errors)
add_subdirectory(il)
add_subdirectory(type_system)

add_subdirectory("variable")
add_subdirectory(ctrl_flow_graph)
add_subdirectory(generators)
add_subdirectory(enviroment)

add_library(il_gen INTERFACE)
target_link_libraries(il_gen INTERFACE il_gen_enviroment il_gen_generators)