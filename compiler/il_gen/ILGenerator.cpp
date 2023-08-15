#include "Compiler.h"
#include "Parser.h"
#include "Lexer.h"
#include <variant>
#include "VectorUtil.h"
#include "spdlog/spdlog.h"
#include "GraphDominance.h"
#include "ExprInterpreter.h"


ILGenerator::ILGenerator()
	: typeSystem(env)
{
}

auto ILGenerator::generate(Stmt::Program input) -> IL::Program
{
	IL::Program ilProgram;
	for (auto& stmt : program) {
		util::vector_append(ilProgram, visitStmt(stmt));
	}
	return ilProgram;
}

auto ILGenerator::getILDestination(std::vector<IL::UniquePtr>& il) -> IL::ILDestination
{
	return il.back()->getFinalDestination();
}

void ILGenerator::visit(Stmt::Bin& bin)
{
	typeSystem.addBin(bin);
	returnForStmt();
}

void ILGenerator::visit(Stmt::Function& func)
{
	enterFunction();
	typeSystem.addFunc(func);
	if (!func.isTemplate()) {
		std::vector<IL::Function::Param> params;
		for (auto& param : func.params) {
			auto type = typeSystem.instantiateType(param.type);
			auto type_size = type.size();
			auto ilVar = createVariable();
			env.variables.emplace(param.name, std::move(EnvInfo(type, ilVar)));
			params.push_back(IL::Function::Param{ type_size, ilVar });
		}
		IL::Type retType = typeSystem.instantiateType(func.retType).size();
		std::vector<IL::UniquePtr> stmts;
		beginNode();
		for (auto& stmt : func.body) {
			util::vector_append(currentNode->stmts, visitStmt(stmt));
		}
		for (auto& frontier : calculateDominanceFrontiers(entryNode)) {
			for (auto& var : frontier.first->usedVariables) {
				if (auto envInfo = env.variables.find(var); envInfo != env.variables.end()) {
					for (auto& node : frontier.second) {
						IL::Type ilType = envInfo->second.type.size();
						auto newPhiNode = IL::makeIL<IL::Phi>(envInfo->second.currentILVar, ilType, );
						node->stmts.insert(node->stmts.begin(), );
					}
				}
			}
		}
		returnForStmt(
			util::combine_vectors(IL::makeIL<IL::Function>(
				func.name, std::move(params), retType, 
				buildNode(), func.isExported
			))
		);
	}
	else {
		returnForStmt();
	}
	exitFunction();
}
void ILGenerator::visit(Stmt::VarDef& varDef)
{
	std::visit([&](auto&& decl) {
		using U = std::remove_cvref_t<decltype(decl)>;
		if constexpr (std::is_same_v<U, Stmt::TypeDecl>) {
			if (!varDef.initializer.has_value()) {
				throw;
			}
			typeSystem.addAlias(decl.name, typeSystem.instantiateType(varDef.initializer.value()));
			returnForStmt();
		}
		else if (std::is_same_v<U, Stmt::VarDecl>) {
			auto type = typeSystem.instantiateType(decl.type);
			if (varDef.initializer.has_value()) {
				auto stmts = visitExpr(varDef.initializer.value());
				auto def = IL::Definition(
					createVariable(), IL::Type(type.size()), getILDestination(stmts).dest
				);
				currentNode->variableToILVariable.emplace(decl.name, def.dest);
				env.variables.emplace(decl.name, EnvInfo{ std::move(type), def.dest });
				returnForStmt(util::combine_vectors(std::move(stmts), IL::makeIL<IL::Definition>(def)));
			}
		}
	}, varDef.decl);
}
void ILGenerator::visit(Stmt::Assign& assign)
{
	auto rhs = visitExpr(assign.rhs);
	auto rhs_dest = getILDestination(rhs);
	assert(dynamic_cast<Expr::Identifier*>(assign.lhs.get()) != nullptr && "Lhs is not an expr");
	auto name = static_cast<Expr::Identifier*>(assign.lhs.get())->ident;
	auto operation = IL::makeIL<IL::Definition>(env.variables.at(name).currentILVar, rhs_dest.type, rhs_dest.dest);
	returnForStmt(util::combine_vectors(std::move(rhs), std::move(operation)));
}

// control flow
void ILGenerator::visit(Stmt::Label& label)
{
	auto brokenFlow = createNode();
	env.labels[label.label] = brokenFlow->startLabel.name;
	currentNode->children.push_back(brokenFlow);
	currentNode->stmts.push_back(IL::makeIL<IL::Jump>(brokenFlow->startLabel));
	currentNode = brokenFlow;
	returnForStmt();
}
void ILGenerator::visit(Stmt::CountLoop& loop)
{
	util::vector_append(currentNode->stmts, visitExpr(loop.initializer));
	auto initialVar = createVariable();
	currentNode->stmts.push_back(IL::makeIL<IL::Definition>(initialVar, IL::Type(2), getILDestination(currentNode->stmts).dest));
	jumpToNewNode();
	for (auto& stmt : loop.body) {
		util::vector_append(currentNode->stmts, visitStmt(stmt));
	}
	jumpToNewNode();
	//util::vector_append(,loop.counter)
		//jumpToNewNode();
	returnForStmt();
}
void ILGenerator::visit(Stmt::If& ifStmt)
{
	auto endNode = createNode();
	auto falseBranch = conditional(currentNode, endNode, ifStmt.ifBranch);
	for (auto& elif : ifStmt.elseIfBranch) {
		falseBranch = conditional(falseBranch, endNode, elif);
	}
	if (ifStmt.elseBranch.has_value()) {
		auto& elseStmts = ifStmt.elseBranch.value();
		falseBranch->stmts.reserve(elseStmts.size());
		for (auto& stmt : elseStmts) {
			util::vector_append(falseBranch->stmts, visitStmt(stmt));
		}
		falseBranch->stmts.push_back(IL::makeIL<IL::Jump>(endNode->startLabel));
		falseBranch->children.push_back(endNode);
	}
	currentNode = endNode;
	returnForStmt();
}

auto ILGenerator::conditional(IL::NodePtr& testingNode, IL::NodePtr& endNode, Stmt::Conditional& condition)->IL::NodePtr
{
	currentNode = testingNode;
	auto [trueNode, falseNode] = splitNode();
	auto conditionStmts = visitExpr(condition.expr);
	auto dest = getILDestination(conditionStmts);
	util::vector_append(testingNode->stmts, std::move(conditionStmts));
	testingNode->stmts.push_back(IL::makeIL<IL::Test>(dest.dest, trueNode->startLabel, falseNode->startLabel));
	
	//the current node must be set to true branch while we build
	currentNode = trueNode;
	for (auto& stmt : condition.body) {
		util::vector_append(currentNode->stmts, visitStmt(stmt));
	}
	currentNode->stmts.push_back(IL::makeIL<IL::Jump>(endNode->startLabel));
	currentNode->children.push_back(endNode);
	return falseNode;
}

void ILGenerator::visit(Stmt::ExprStmt& exprStmt)
{
	returnForStmt(visitExpr(exprStmt.expr));
}
void ILGenerator::visit(Stmt::Return& stmt)
{
	auto stmts = visitExpr(stmt.expr);
	auto prev_dest = getILDestination(stmts);
	returnForStmt(util::combine_vectors(std::move(stmts), IL::makeIL<IL::Return>(prev_dest.type, prev_dest.dest)));
}

void ILGenerator::visit(Stmt::Instruction& stmt)
{
	returnForStmt(util::combine_vectors(IL::makeIL<IL::Instruction>(std::move(stmt))));
}

void ILGenerator::visit(Expr::Binary& expr)
{
	auto lhs = visitExpr(expr.lhs);
	auto rhs = visitExpr(expr.rhs);
	auto lhs_dest = getILDestination(lhs);
	auto rhs_dest = getILDestination(rhs);
	//assert(rhs_dest.type == lhs_dest.type);
	auto variable = createVariable();
	auto operation = IL::makeIL<IL::Binary>(variable, lhs_dest.type, lhs_dest.dest, expr.oper, rhs_dest.dest);
	returnForExpr(util::combine_vectors(std::move(lhs), std::move(rhs), std::move(operation)));
}
void ILGenerator::visit(Expr::Unary& expr)
{
	auto lhs = visitExpr(expr.expr);
	auto lhs_dest = getILDestination(lhs);
	auto variable = createVariable();
	auto operation = IL::makeIL<IL::Unary>(variable, lhs_dest.type, lhs_dest.dest, expr.oper);
	returnForExpr(util::combine_vectors(std::move(lhs), std::move(operation)));
}

void ILGenerator::visit(Expr::Parenthesis& expr)
{
	returnForExpr(visitExpr(expr.expr));
}

void ILGenerator::visit(Expr::Identifier& expr)
{
	/*
	auto& varToPhi = currentNode->variablePhiNode;
	IL::Type ilType = env.variables.at(expr.ident).type.size();
	if (auto it = varToPhi.find(expr.ident); it == varToPhi.end()) {
		auto phiVar = createVariable();
		auto phiIL = std::make_unique<IL::Phi>(phiVar, ilType, std::vector<IL::Phi::NodeSource>{});
		varToPhi[expr.ident] = phiIL.get();
		currentNode->variableToILVariable.emplace(expr.ident, phiVar);
		returnForExpr(util::combine_vectors(std::unique_ptr<IL::IL>(std::move(phiIL))));
	}
	//else {
		IL::Variable oldSrc = currentNode->variableToILVariable.at(expr.ident);
		returnForExpr(util::combine_vectors(IL::makeIL<IL::Definition>(createVariable(), ilType, oldSrc)));
	//}*/
	IL::Type ilType = env.variables.at(expr.ident).type.size();
	IL::Variable oldSrc = env.variables.at(expr.ident).currentILVar;
	returnForExpr(util::combine_vectors(IL::makeIL<IL::Definition>(createVariable(), ilType, oldSrc)));
}

void ILGenerator::visit(Expr::MemberAccess& expr)
{
	returnForExpr();
}

void ILGenerator::visit(Expr::Literal& expr)
{
	std::visit([&](auto&& arg) {
		using U = std::remove_cvref_t<decltype(arg)>;
		if constexpr (std::is_same_v<U, std::string>) {
			returnForExpr(util::combine_vectors(IL::makeIL<IL::Definition>(createVariable(), IL::Type(IL::DataType::str, true), arg)));
		}
		else if constexpr (std::is_same_v<U, u16>) {
			returnForExpr(util::combine_vectors(IL::makeIL<IL::Definition>(createVariable(), IL::Type(IL::DataType::i16, false), arg)));
		}
		else {
			throw;
		}
	}, expr.literal);
}

void ILGenerator::visit(Expr::CurrentPC& expr)
{
	returnForExpr(util::combine_vectors(IL::makeIL<IL::Definition>(createVariable(), IL::Type(IL::DataType::i16), IL::PC{})));
}

void ILGenerator::visit(Expr::Register& expr)
{//err
}

void ILGenerator::visit(Expr::Flag& expr)
{//err
}


void ILGenerator::visit(Stmt::Module& mod)
{
}

void ILGenerator::visit(Stmt::Import& imp)
{
}

void ILGenerator::visit(Expr::FunctionCall& expr)
{

}

void ILGenerator::visit(Expr::TemplateCall& expr)
{
}

void ILGenerator::visit(Expr::Indexing& expr)
{
}