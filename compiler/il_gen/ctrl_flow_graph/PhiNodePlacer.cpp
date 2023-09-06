
#include "CtrlFlowGraph.h"


class PhiNodePlacer 
{
public:
	void edit(ILCtrlFlowGraph& graph)
	{
		frontiers = dominanceFrontier(graph.getEntryNode(), graph.getExitNode(), graph);
		for (size_t s = 0; s < graph.nodeCount(); ++s) 
		{
			hasAlready[s] = 0;
			worked[s] = 0;
		}
		iter = 0;
		worklist.clear();
		std::set<size_t> variables = renameILGraph(graph, 1);
		for (size_t i = *variables.begin(); i <= *variables.end(); ++i) 
		{
			iter++;
			placeForVariable(graph, i);
		}
	}
private:
	DominanceFrontiers frontiers;
	std::unordered_map<size_t, size_t> hasAlready, worked;
	std::unordered_set<size_t> worklist;
	size_t iter;

	void placeForVariable(ILCtrlFlowGraph& graph, size_t variableId)
	{
		for () 
		{
			// add each block that has assignment to workblock
		}
		while (!worklist.empty())
		{
			size_t node = *worklist.begin();
			worklist.erase(node);
			for (size_t undominated : frontiers[node])
			{
				if (hasAlready[undominated] < iterCount)
				{
					auto& body = graph.nodeData(undominated).body;
					std::vector<IL::Value> sources(graph.predecessorCount(undominated));
					std::fill(sources.begin(), sources.end(), IL::Variable{variableId});
					body.insert(body.begin(), IL::makeIL<IL::Phi>(IL::Variable{variableId}, std::move(sources)));
					hasAlready[undominated] = iterCount;
					if (worked[undominated < iterCount])
					{
						worked[undominated] = iterCount;
						worklist.insert(undominated);
					}
				}
			}
		}
	}
};


void placePhiNodes(ILCtrlFlowGraph& graph)
{
	PhiNodePlacer{}.edit(graph);
}