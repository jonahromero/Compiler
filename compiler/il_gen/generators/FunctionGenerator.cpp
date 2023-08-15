#include "FunctionGenerator.h"
#include "SemanticError.h"
#include "CFGGenerator.h"
#include "VectorUtil.h"
#include "ExprGenerator.h"
#include "CtrlFlowGraphFlattener.h"

FunctionGenerator::FunctionGenerator(Enviroment& env)
	: env(env)
{
}

IL::Function FunctionGenerator::generate(Stmt::Function function)
{
	ILCtrlFlowGraph ilCfg;
	env.startScope();
	{
		startFunctionEnviroment(function.name, function.params, function.retType);
		ilCfg = transformGraph(CtrlFlowGraphGenerator{ function.body }.generate());
	}
	env.endScope();

	IL::Function function;
	function.body = flattenILCtrlFlowGraph(std::move(ilCfg));
	return function;
}

void FunctionGenerator::startFunctionEnviroment(std::string_view name, std::vector<Stmt::VarDecl> const& params, Expr::UniquePtr const& returnType)
{
	TypeInstance instantiatedReturnType;
	std::vector<TypeInstance> instantiatedParamTypes;
	IL::Function::Signature signature = createFunctionSignature(params, returnType, instantiatedParamTypes, instantiatedReturnType);
	TypePtr type = env.types.addFunction(std::move(instantiatedParamTypes), instantiatedReturnType);
	env.createVariable(name, TypeInstance(type));
}

IL::Function::Signature FunctionGenerator::createFunctionSignature(
	std::vector<Stmt::VarDecl> const& params, Expr::UniquePtr const& returnType, 
	std::vector<TypeInstance>& compiledParamTypes, TypeInstance& instantiatedReturnType
) 
{
	IL::Function::Signature signature;
	instantiatedReturnType = env.instantiateType(returnType);
	signature.params = util::transform_vector(params, [&](Stmt::VarDecl const& decl) {
		compiledParamTypes.push_back(env.instantiateType(decl.type));
		IL::Variable var = env.createVariable(decl.name, compiledParamTypes.back());
		TypeInstance type = env.getILAliasType(var);
		return IL::Decl(var, type);
	});
	// sort the parameters such that pointers come at the end
	std::sort(signature.params.begin(), signature.params.end(), [](auto& lhs, auto& rhs) {
		return (lhs == IL::Type::u8_ptr) < (lhs == IL::Type::u8_ptr);
	});
	auto compiledReturnType = env.types.compileType(instantiatedReturnType);
	if (compiledReturnType != IL::Type::u8_ptr) {
		signature.returnType = compiledReturnType;
	}
	else {
		signature.params.push_back(IL::Type::u8_ptr);
		signature.returnType = IL::Type::void_;
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
			auto exprResult = ExprGenerator::defaultContext(env).generateWithCast(uncompiledNode.splitsOn(), IL::Type::i1);
			util::vector_append(outBody, std::move(exprResult.instructions));
			out.nodeData(node).splitWith(exprResult.output);
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
	IL::Program output;

	std::visit([&](auto& decl) {
		using U = std::remove_cvref_t<decltype(decl)>;
		if constexpr (std::is_same_v<U, Stmt::VarDecl>) {
			IL::Variable ilVar = env.createVariable(decl.name, env.instantiateType(decl.type));
			IL::Type ilType = env.getILAliasType(ilVar);
			if (ilType == IL::Type::u8_ptr) {
				output.push_back(IL::makeIL<IL::Allocate>(ilVar, env.getVariableType(decl.name).type->size));
			}
			if (varDef.initializer.has_value()) {
				auto exprResult = ExprGenerator::typedContext(env, ilType).generateWithCast(varDef.initializer.value(), ilType);
				util::vector_append(output, std::move(exprResult.instructions));
				output.push_back(IL::makeIL<IL::Assignment>(
					ilVar, ilType, exprResult.output
				));
			}
		}
		else if (std::is_same_v<U, Stmt::TypeDecl>) {
			if (!varDef.initializer.has_value()) {
				throw SemanticError(varDef.sourcePos, "Type Alias must have an initializer");
			}
			env.addTypeAlias(decl.name, env.instantiateType(varDef.initializer.value()));
		}
	}, varDef.decl);

	returnValue(std::move(output));
}

void FunctionGenerator::visit(Stmt::Assign& assign)
{
	ILExprProduct lhs = ExprGenerator::defaultContext(env).generate(assign.lhs);
	if (!lhs.producesObjectRef()) {
		throw SemanticError(assign.sourcePos, "Left hand side does not produce a l-value to assign to.");
	}
	TypeInstance refType = lhs.getResultingRefType();
	IL::Type ilType = env.types.compileType(refType.type);
	if (!refType.isMut) {
		throw SemanticError(assign.sourcePos, "Unable to assign a value to a non-mutable reference.");
	}
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
	//returnValue(ExprGenerator::defaultContext(env).generate(stmt.expr));
}
