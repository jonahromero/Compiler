#include "FunctionGenerator.h"
#include "SemanticError.h"
#include "CFGGenerator.h"
#include "VectorUtil.h"
#include "VariantUtil.h"
#include "ExprGenerator.h"

FunctionGenerator::FunctionGenerator(Enviroment& env)
	: gen::Generator(env), env(env)
{
}

IL::Function FunctionGenerator::generate(Stmt::Function function)
{
	IL::ILBody instructions;
	FunctionEnviroment functionEnv{ env };
	functionEnv.addParameters(instructions, function.params);

	if (function.retType.has_value()) 
	{
		TypeInstance returnType = env.instantiateType(function.retType.value());
		returnVariable = allocateNonPossessingVariable(instructions, returnType);
	}
	ILCtrlFlowGraph ilCfg = transformGraph(CtrlFlowGraphGenerator{ function.body }.generate());
	util::vector_append(instructions, flattenILCtrlFlowGraph(std::move(ilCfg)));

	return IL::Function{
		function.name,
		functionEnv.determineILSignature(returnVariable),
		function.isExported,
		std::move(instructions)
	};
}

ILCtrlFlowGraph FunctionGenerator::transformGraph(CtrlFlowGraph graph)
{
	ILCtrlFlowGraph out = graph.shape();
	graph.bfs(graph.getEntryNode(), [&](size_t node) 
	{
		if (graph.nodeData(node).isTrueBranch()) {
			out.nodeData(node) = ILBlock::trueBlock();
		}
		else if (graph.nodeData(node).isFalseBranch()) {
			out.nodeData(node) = ILBlock::falseBlock();
		}
		else {
			out.nodeData(node) = ILBlock::defaultBlock();
		}
		auto& outBody = out.nodeData(node).body;
		auto& uncompiledNode = graph.nodeData(node);
		for (auto& stmt : uncompiledNode.body) {
			util::vector_append(outBody, visitChild(stmt));
		}
		if (uncompiledNode.splits()) {
			TypeInstance boolType = env.types.getPrimitiveType(PrimitiveType::SubType::bool_);
			auto exprResult = ExprGenerator::defaultContext(env).generateWithCast(uncompiledNode.splitsOn(), boolType);
			util::vector_append(outBody, std::move(exprResult.instructions));
			out.nodeData(node).splitWith(exprResult.output.ilName);
		}
	});
	return out;
}

// Control Flow Graph will have filtered these types of statements out. 
// It is an internal logic error, if any of these are called
void FunctionGenerator::visit(Stmt::Bin& bin) {}
void FunctionGenerator::visit(Stmt::Module& mod) {}
void FunctionGenerator::visit(Stmt::Import& imp) {}
void FunctionGenerator::visit(Stmt::Function& func) {}
void FunctionGenerator::visit(Stmt::CountLoop& loop) {}
void FunctionGenerator::visit(Stmt::If& ifStmt) {}
void FunctionGenerator::visit(Stmt::Label& label) {}


void FunctionGenerator::visit(Stmt::VarDef& varDef)
{
	IL::Program instructions;

	std::visit(util::OverloadVariant
	{
	[&](Stmt::VarDecl const& decl) 
	{
		TypeInstance type = env.instantiateType(decl.type);
		gen::Variable lhs = allocateVariable(instructions, type);
		env.registerVariableName(decl.name, lhs);

		if (varDef.initializer.has_value()) 
		{
			auto exprResult = ExprGenerator::typedContext(env, type.type)
											.generateWithCast(varDef.initializer.value(), type);
			util::vector_append(instructions, std::move(exprResult.instructions));
			assignVariable(instructions, lhs, exprResult.output);
		}
	},
	[&](Stmt::TypeDecl const& decl)
	{
		if (!varDef.initializer.has_value()) {
			throw SemanticError(varDef.sourcePos, "Type Alias must have an initializer");
		}
		env.addTypeAlias(decl.name, env.instantiateType(varDef.initializer.value()));
	}
	}, varDef.decl);

	returnValue(std::move(instructions));
}

void FunctionGenerator::visit(Stmt::Assign& assign)
{
	IL::Program instructions;
	ILExprResult lhs = ExprGenerator::defaultContext(env).generate(assign.lhs);
	ILExprResult rhs = ExprGenerator::defaultContext(env).generate(assign.rhs);
	util::vector_append(instructions, std::move(lhs.instructions));
	util::vector_append(instructions, std::move(rhs.instructions));

	if (lhs.isTemporary()) {
		throw SemanticError(assign.sourcePos, "Left hand side does not produce a l-value to assign to.");
	}
	if (!lhs.output.type.isMut) {
		throw SemanticError(assign.sourcePos, "Cannot assign a value as a mutable reference.");
	}
	assignVariable(instructions, lhs.output, rhs.output);
	returnValue(std::move(instructions));
}

void FunctionGenerator::visit(Stmt::ExprStmt& exprStmt)
{
	returnValue(ExprGenerator::defaultContext(env).generate(exprStmt.expr).instructions);
}

void FunctionGenerator::visit(Stmt::Instruction& stmt)
{
	IL::Program single;
	single.push_back(IL::makeIL<IL::Instruction>(std::move(stmt)));
	returnValue(std::move(single));
}

void FunctionGenerator::visit(Stmt::Return& stmt)
{
	IL::Program instructions;
	auto result = ExprGenerator::defaultContext(env).generateWithCast(stmt.expr, returnVariable.value().type);
	if (!returnVariable.has_value()) 
	{
		returnVariable = allocateNonPossessingVariable(instructions, result.output.type);
	}
	assertIsAssignableType(stmt.sourcePos, result.output.type, returnVariable.value().type);
	assignVariable(instructions, returnVariable.value(), result.output);
	result.instructions.push_back(IL::makeIL<IL::Return>(returnVariable.value().ilName));
	returnValue(std::move(result.instructions));
}
