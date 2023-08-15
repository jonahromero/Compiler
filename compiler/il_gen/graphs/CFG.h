#include "Graph.h"
#include "Stmt.h"

struct CFGBlock {
    enum class BranchType { NONE, FALSE_BRANCH, TRUE_BRANCH } branchType;
    std::optional<std::string_view> label;
    Stmt::StmtBody body;
};

struct CFG : public Graph<CFGBlock>
{
public:
    CFG() {
        // create entry and exit nodes
        addCFBlock({}); addCFBlock({});
    }
    size_t getEntryNode() { return 0; }
    size_t getExitNode() { return 1; }
};
