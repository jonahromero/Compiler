#include "CtrlFlowGraph.h"
#include <set>

class ILRenamer
	: public ::IL::Visitor
{
	using RenameTable = std::unordered_map<IL::Variable, IL::Variable>;
public:
	ILRenamer(size_t startId) : startId(startId) {}

	void rename(IL::UniquePtr& expr) 
	{
		this->::IL::Visitor::visitChild(expr);
	}

	void rename(IL::Variable& var)
	{
		if (renameTable.count(var) == 0)
		{
			renameTable.emplace(var, IL::Variable(startId++));
		}
		var = renameTable.at(var);
	}

	std::set<size_t> variables()
	{
		std::set<size_t> vars;
		for (auto& [old, updated] : renameTable)
		{
			vars.insert(updated.id);
		}
		return vars;
	}

	virtual void visit(IL::Binary& expr) override 
	{
		rename(expr.dest.variable);
		rename(expr.lhs);
		rename(expr.rhs);
	}
	virtual void visit(IL::Unary& expr) override
	{
		rename(expr.dest.variable);
		rename(expr.src);
	}
	//Primary expressions
	virtual void visit(IL::Function& func) override 
	{
		for (auto& param : func.signature.params) 
		{
			rename(param.variable);
		}
		for (auto& line : func.body) 
		{
			rename(line);
		}
	}
	virtual void visit(IL::TestBit& expr) override 
	{
		rename(expr.dest);
		rename(expr.src);
	}
	virtual void visit(IL::Test& expr) override 
	{
		rename(expr.var);
	}
	virtual void visit(IL::Phi& expr) override 
	{
		rename(expr.dest);
		for (auto& source : expr.sources)
		{
			rename(source);
		}
	}
	virtual void visit(IL::Return& expr) override 
	{
		if(expr.value.has_value())
			rename(expr.value.value());
	}
	virtual void visit(IL::Assignment& expr) override 
	{
		rename(expr.dest.variable);
		rename(expr.src);
	}
	virtual void visit(IL::FunctionCall& call) override
	{
		rename(call.dest);
		rename(call.args);
		if (std::holds_alternative<IL::Variable>(call.function)) 
		{
			rename(std::get<IL::Variable>(call.function));
		}
	}
	virtual void visit(IL::Cast& cast) override 
	{
		rename(cast.dest);
		rename(cast.src);
	}
	virtual void visit(IL::Allocate& allocation) override 
	{
		rename(allocation.dest);
	}
	virtual void visit(IL::AddressOf& addressOf) override
	{
		rename(addressOf.ptr);
		if (std::holds_alternative<IL::Variable>(addressOf.target))
		{
			rename(std::get<IL::Variable>(addressOf.target));
		}
	}
	virtual void visit(IL::Deref& deref) override 
	{
		rename(deref.ptr);
		rename(deref.dest.variable);
		rename(deref.ptr);
	}
	virtual void visit(IL::Store& store) override 
	{
		rename(store.src.variable);
		rename(store.ptr);
	}
	virtual void visit(IL::MemCopy& copy) override 
	{
		rename(copy.dest);
		rename(copy.src);
	}
	virtual void visit(IL::Instruction& expr) override {}
	virtual void visit(IL::Jump& jump) override {}
	virtual void visit(IL::Label& label) override {}
private:
	RenameTable renameTable;
	size_t startId;

	void rename(IL::Value& value) 
	{
		if (std::holds_alternative<IL::Variable>(value)) 
		{
			rename(std::get<IL::Variable>(value));
		}
	}
	void rename(IL::Decl& decl) 
	{
		rename(decl.variable);
	}
	template<typename T>
	void rename(std::vector<T>& list) 
	{
		for (auto& elem : list) rename(elem);
	}
};

std::set<size_t> renameILGraph(ILCtrlFlowGraph& graph, size_t startId) 
{
	ILRenamer renamer{ startId };
	graph.bfs(graph.getEntryNode(), [&](size_t node) 
	{
		auto& nodeData = graph.nodeData(node);
		for (IL::UniquePtr& data : nodeData.body) 
		{
			renamer.rename(data);
		}
		if (nodeData.splits()) {
			IL::Variable oldSplit = nodeData.splitsOn();
			renamer.rename(oldSplit);
			nodeData.splitWith(oldSplit);
		}
	});
	return renamer.variables();
}