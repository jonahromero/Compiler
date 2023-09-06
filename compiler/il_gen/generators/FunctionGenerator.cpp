#include "FunctionGenerator.h"
#include "SemanticError.h"
#include "CFGGenerator.h"
#include "VectorUtil.h"
#include "VariantUtil.h"
#include "ExprGenerator.h"
#include <iostream>

FunctionGenerator::FunctionGenerator(Enviroment& env, IL::Program& moduleInstructions)
	: gen::GeneratorToolKit(env), env(env), moduleInstructions(moduleInstructions)
{
}

IL::Function FunctionGenerator::generate(Stmt::Function function)
{
	auto [ilFunction, paramTypes] = generate_(std::move(function));
	TypePtr funcType = env.types.addFunction(paramTypes, returnVariable.value().type);
	gen::Variable address = getPointerTo(moduleInstructions, IL::AddressOf::Function{function.name}, funcType->getExactType<FunctionType>());
	env.registerVariableName(function.name, address);
	return std::move(ilFunction);
}

ILCtrlFlowGraph FunctionGenerator::transformGraph(CtrlFlowGraph graph)
{
	ILCtrlFlowGraph out = graph.shape();
	auto& entryBody = out.nodeData(out.getEntryNode()).body;
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
		for (auto& stmt : uncompiledNode.body) 
		{
			auto blockStmtResult = visitChild(stmt);
			util::vector_append(outBody, std::move(blockStmtResult.instructions));
			util::vector_append(entryBody, std::move(blockStmtResult.allocations));
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

std::pair<IL::Function, std::vector<TypeInstance>> FunctionGenerator::generate_(Stmt::Function function)
{
	IL::ILBody instructions;
	FunctionEnviroment functionEnv{ env };
	std::vector<TypeInstance> paramTypes = functionEnv.addParameters(instructions, function.params);

	if (function.retType.has_value())
	{
		TypeInstance returnType = env.types.instantiateType(function.retType.value());
		if (shouldPassReturnAsParameter(returnType)) {
			returnVariable = allocateNonPossessingVariable(instructions, returnType);
		}
		else {
			returnVariable = allocateVariable(instructions, returnType);
		}
	}
	// DOMINANCE FRONTIER STUFF
	ILCtrlFlowGraph ilCfg = transformGraph(CtrlFlowGraphGenerator{ function.body }.generate());
	//renameILGraph(ilCfg, 1);
	auto dominance = dominanceFrontier(ilCfg.getEntryNode(), ilCfg.getExitNode(), ilCfg);
	std::cout << "Function: " << function.name << std::endl;
	std::cout << ilCfg << std::endl;
	for (auto [dominator, dominated] : dominance) 
	{
		std::cout << "State " << dominator << " dominance frontier: { ";
		bool first = true;
		for (size_t s : dominated)
		{
			if (!first) std::cout << ", ";
			std::cout << s;
			if (first) first = false;
		}
		std::cout << " }\n" << std::endl;
	}
	
	//END DOMINANCE
	util::vector_append(instructions, flattenILCtrlFlowGraph(std::move(ilCfg)));

	return std::make_pair(IL::Function{
		function.name,
			functionEnv.determineILSignature(returnVariable),
			function.isExported,
			std::move(instructions)
	}, paramTypes);
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
		TypeInstance type = env.types.instantiateType(decl.type);
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
		env.types.addAlias(decl.name, env.types.instantiateType(varDef.initializer.value()));
	}
	}, varDef.decl);

	returnValue(std::move(instructions));
}

void FunctionGenerator::visit(Stmt::Assign& assign)
{
	IL::Program instructions;
	ILExprResult lhs = ExprGenerator::defaultContext(env).generate(assign.lhs);
	ILExprResult rhs = ExprGenerator::typedContext(env, lhs.output.type.type).generate(assign.rhs);
	util::vector_append(instructions, std::move(lhs.instructions));
	util::vector_append(instructions, std::move(rhs.instructions));

	if (lhs.isTemporary()) {
		throw SemanticError(assign.sourcePos, "Left hand side does not produce a l-value to assign to.");
	}
	if (!lhs.output.type.isMut) {
		throw SemanticError(assign.sourcePos, "Cannot assign to a value with non-mutable type.");
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
	auto exprGenerator = returnVariable.has_value() ?
		ExprGenerator::typedContext(env, returnVariable.value().type.type) :
		ExprGenerator::defaultContext(env);
	auto result = exprGenerator.generate(stmt.expr);
	util::vector_append(instructions, std::move(result.instructions));

	if (!returnVariable.has_value()) 
	{
		if (shouldPassReturnAsParameter(result.output.type)) {
			returnVariable = allocateNonPossessingVariable(instructions, result.output.type);
		}
		else {
			returnVariable = allocateVariable(instructions, result.output.type);
		}
	}
	assertIsAssignableType(stmt.sourcePos, result.output.type, returnVariable.value().type);
	assignVariable(instructions, returnVariable.value(), result.output);
	if (shouldPassReturnAsParameter(returnVariable.value().type)) 
	{	
		instructions.push_back(IL::makeIL<IL::Return>());
	}
	else
	{
		instructions.push_back(IL::makeIL<IL::Return>(returnVariable.value().ilName));
	}
	returnValue(std::move(instructions));
}
