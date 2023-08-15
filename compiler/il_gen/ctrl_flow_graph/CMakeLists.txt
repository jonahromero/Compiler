
add_library(il_gen_ctrl_flow_graph STATIC 
	GraphDominance.cpp
	CtrlFlowGraphFlattener.cpp
)

target_link_libraries(il_gen_ctrl_flow_graph PUBLIC il util)
target_include_directories(il_gen_ctrl_flow_graph PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")