
#pragma once
#include <string>
#include "Graph.h"
#include "Stmt.h"
#include "IL.h"
#include "CompilerError.h"
#include "VectorUtil.h"

template<typename BodyType, typename ConditionType>
class GenericBlock
{
    template<typename>
    friend class GenericCtrlFlowGraph;
    using ThisType = GenericBlock<BodyType, ConditionType>;
public:
    GenericBlock() { *this = defaultBlock(); }
    static ThisType defaultBlock(std::string_view label = "") { return ThisType(BranchType::NONE, label); }
    static ThisType falseBlock(std::string_view label = "") { return ThisType(BranchType::FALSE_BLOCK, label); }
    static ThisType trueBlock(std::string_view label = "") { return ThisType(BranchType::TRUE_BLOCK, label); }

    bool isNamed() const { return label != ""; }
    bool splits() const { return splitExpr.has_value(); }
    bool isTrueBranch() const { return branchType == BranchType::TRUE_BLOCK; }
    bool isFalseBranch() const { return branchType == BranchType::FALSE_BLOCK; }

    void name(std::string_view name) { label = name; }
    std::string_view getName() const { return label; }
    void splitWith(ConditionType expr) { splitExpr = std::move(expr); }
    ConditionType const& splitsOn() const { return splitExpr.value(); }
    ConditionType& splitsOn() { return splitExpr.value(); }

    BodyType body;

private:
    enum class BranchType : short { 
        NONE, FALSE_BLOCK, TRUE_BLOCK 
    };

    GenericBlock(BranchType branchType, std::string_view label)
        : label(label), branchType(branchType) {}

    std::string_view label;
    std::optional<ConditionType> splitExpr;
    BranchType branchType = BranchType::NONE;
};

struct CtrlFlowGraphShape {
    PureGraph const& graph;
};

template<typename BlockType>
class GenericCtrlFlowGraph : public Graph<BlockType>
{
public:
    GenericCtrlFlowGraph() {
        // create entry and exit nodes
        this->createNode(BlockType::defaultBlock());
        this->createNode(BlockType::defaultBlock());
    }
    GenericCtrlFlowGraph(CtrlFlowGraphShape other)
        : Graph<BlockType>(other.graph) {
    }

    size_t getEntryNode() const { return 0; }
    size_t getExitNode() const { return 1; }
    CtrlFlowGraphShape shape() const { return CtrlFlowGraphShape{*static_cast<const PureGraph*>(this) }; }

    size_t getTrueSuccessor(size_t node) const {
        for (auto succ : this->out(node)) {
            if (this->nodeData(succ).isTrueBranch())
                return succ;
        }
        COMPILER_NOT_REACHABLE;
    }
    
    size_t getFalseSuccessor(size_t node) const {
        for (auto succ : this->out(node)) {
            if (this->nodeData(succ).isFalseBranch())
                return succ;
        }
        COMPILER_NOT_REACHABLE;
    }

    size_t getSuccessor(size_t node) const {
        for (auto succ : this->out(node)) return succ;
        COMPILER_NOT_REACHABLE;
    }
};

using Block = GenericBlock<Stmt::StmtBody, Expr::UniquePtr>;
using ILBlock = GenericBlock<IL::ILBody, IL::Variable>;

using CtrlFlowGraph = GenericCtrlFlowGraph<Block>;
using ILCtrlFlowGraph = GenericCtrlFlowGraph<ILBlock>;

std::unordered_map<size_t, std::vector<size_t>> dominanceFrontier(CtrlFlowGraph const& graph);
IL::Program flattenILCtrlFlowGraph(ILCtrlFlowGraph graph);