
#include "CtrlFlowGraph.h"
#include <set>

class CtrlFlowGraphFlattener
{
public:
    IL::Program flatten(ILCtrlFlowGraph graph)&&;

private:
    void addBlock(ILCtrlFlowGraph& graph, size_t node);

    IL::Program program;
    std::unordered_map<size_t, IL::Label> currentLabels;
    std::set<size_t> visited;
    size_t labelCounter = 0;
};

IL::Program CtrlFlowGraphFlattener::flatten(ILCtrlFlowGraph graph) &&
{
    graph.bfs(graph.getEntryNode(), [&](size_t node) {
        auto& block = graph.nodeData(node);
        if (block.isTrueBranch()) {
            currentLabels.emplace(node, IL::Label(labelCounter++));
        }
    });
    addBlock(graph, graph.getEntryNode());
    return std::move(program);
}

void CtrlFlowGraphFlattener::addBlock(ILCtrlFlowGraph& graph, size_t node)
{
    if (node == graph.getExitNode()) return;
    if (visited.count(node) != 0) return;
    else visited.insert(node);

    ILBlock& block = graph.nodeData(node);
    if (currentLabels.count(node) != 0) {
        program.push_back(IL::makeIL<IL::Label>(currentLabels.at(node).name));
    }
    util::vector_append(program, std::move(block.body));

    if (block.splits())
    {
        auto trueSuccessor  = graph.getTrueSuccessor(node);
        auto falseSuccessor = graph.getFalseSuccessor(node);
        program.push_back(IL::makeIL<IL::Test>(
            block.splitsOn(), currentLabels.at(trueSuccessor))
        );
        auto endLabel = IL::Label(labelCounter++);
        addBlock(graph, falseSuccessor);
        program.push_back(IL::makeIL<IL::Jump>(endLabel));
        addBlock(graph, trueSuccessor);
        program.push_back(IL::makeIL<IL::Label>(endLabel.name));
    }
    else {
        addBlock(graph, graph.getSuccessor(node));
    }
}

IL::Program flattenILCtrlFlowGraph(ILCtrlFlowGraph graph)
{
    return CtrlFlowGraphFlattener().flatten(std::move(graph));
}
