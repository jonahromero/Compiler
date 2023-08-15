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
	IL::Function ilFunction;
	ILCtrlFlowGraph ilCfg;
	env.startScope();
	{	
		std::vector<gen::Variable> paramVariables = allocateSignature(ilFunction.body, function.params, function.retType);
		ilCfg = transformGraph(CtrlFlowGraphGenerator{ function.body }.generate());
	}
	env.endScope();

	ilFunction.body = flattenILCtrlFlowGraph(std::move(ilCfg));
	ilFunction.signature = createFunctionSignature(paramVariables, returnVariable);
	return ilFunction;
}

std::vector<gen::Variable> 
FunctionGenerator::allocateSignature(
	IL::Program& instructions,
	std::vector<Stmt::VarDecl> const& params, 
	std::optional<Expr::UniquePtr> const& retType
) const
{
	std::vector<gen::Variable> paramVariables;
	for (auto& param : params) 
	{
		TypeInstance type = env.instantiateType(param.type);
		paramVariables.push_back(allocateParameter(instructions, type);
	}
	if (retType.has_value()) 
	{
		returnVariable = env.instantiateType(retType.value());
		paramVariables.push_back();
	}
	return paramVariables;
}

IL::Function::Signature FunctionGenerator::createFunctionSignature(std::vector<gen::Variable> const& params, std::optional<gen::Variable> const& retType)
{
	IL::Function::Signature signature;
	signature.params = util::transform_vector(params, [&](gen::Variable const& param) {
		return IL::Decl(param.ilName, env.getILAliasType(param.ilName));
	});
	auto ilReturnType = env.getILVariableType(returnVariable.ilName);
	if (!retType.has_value()) {
		signature.returnType = IL::Type::void_;
	}
	else if (shouldPassInReturnValue(retType)) 
	{
		signature.returnType = IL::Type::void_;
		signature.params.push_back(IL::Decl(returnVariable.ilName, ilReturnType));
	}
	else 
	{
		signature.returnType = ilReturnType;
	}
	return signature;
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
		gen::Variable lhs = allocateNamedVariable(instructions, decl.name, type);

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
	IL::ILBody instructions;
	ILExprResult lhs = ExprGenerator::defaultContext(env).generate(assign.lhs);
	ILExprResult rhs = ExprGenerator::defaultContext(env).generate(assign.rhs);
	util::vector_append(instructions, lhs.instructions);
	util::vector_append(instructions, rhs.instructions);

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
	auto result = ExprGenerator::defaultContext(env).generateWithCast(stmt.expr, returnVariable.value().type);
	if (!returnVariable.has_value()) 
	{
		returnVariable = allocateReturnValue(result.output.type);
	}
	assertIsAssignableType(stmt.sourcePos, result.output.type, returnVariable.value().type);
	assignVariable(instructions, returnVariable.value(), result.output);
	result.instructions(IL::makeIL<IL::Return>(returnVariable.value().ilName));
	returnValue(std::move(result.instructions));
}
