
add_library(il_gen_ctrl_flow_graph STATIC 
	"../../util/GraphDominance.cpp"
	CtrlFlowGraphFlattener.cpp
 #comm"PhiNodePlacer.cpp" 
 "Renamer.cpp" 
 "BlockAssignments.cpp")

target_link_libraries(il_gen_ctrl_flow_graph PUBLIC il util errors)
target_include_directories(il_gen_ctrl_flow_graph PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")