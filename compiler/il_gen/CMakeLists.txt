
add_subdirectory(common)
add_subdirectory(generators)
add_subdirectory(enviroment)
add_subdirectory(il)


add_library(il_gen INTERFACE)
target_link_libraries(il_gen INTERFACE il_gen_enviroment il_gen_generators)