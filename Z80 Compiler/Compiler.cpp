#include "Compiler.h"
#include "Parser.h"
#include "Lexer.h"
#include <variant>
#include "VectorUtil.h"
#include "spdlog/spdlog.h"
#include "StmtPrinter.h"

auto Compiler::compile(std::string_view input) -> IL::Program
{
	auto tokens = Lexer(input).generateTokens();
	Stmt::Program program = Parser(tokens).program();
	IL::Program ilProgram;
	for (auto& stmt : program) {
		util::vector_append(ilProgram, visitStmt(stmt));
	}
	return ilProgram;
}

auto Compiler::getILDefinition(std::vector<IL::UniquePtr>& il) -> IL::Definition&
{
	auto lastDefinition = dynamic_cast<IL::Definition*>(il.back().get());
	assert(lastDefinition != nullptr);
	return *lastDefinition;
}

auto Compiler::createVariable() -> IL::Variable
{
	return inGlobalScope ? create_global_variable() : create_variable();
}

void Compiler::visit(Stmt::Bin& bin)
{
	typeSystem.addBin(bin);
	returnForStmt();
}
void Compiler::visit(Stmt::Function& func)
{
	inGlobalScope = false;
	typeSystem.addFunc(func);
	if (!func.isTemplate()) {
		typeSystem.addFunc(func);
		std::vector<IL::Function::Param> params;
		for (auto& param : func.params) {
			//params.push_back(IL::Function::Param{ param.type, param.name });
		}
		std::vector<IL::UniquePtr> stmts;
		for (auto& stmt : func.body) {
			util::vector_append(stmts, visitStmt(stmt));
		}
		returnForStmt(
			util::combine_vectors(IL::makeIL<IL::Function>(
				func.name, std::move(params), 
				typeSystem.instantiateType(env, func.retType).type->size, 
				std::move(stmts), func.isExported
			))
		);
	}
	inGlobalScope = true;
	returnForStmt();
}
void Compiler::visit(Stmt::VarDef& varDef)
{
	std::visit([&](auto&& decl) {
		using U = std::remove_cvref_t<decltype(decl)>;
		if constexpr (std::is_same_v<U, Stmt::TypeDecl>) {
			if (!varDef.initializer.has_value()) {
				throw;
			}
			typeSystem.addAlias(decl.name, typeSystem.instantiateType(env, varDef.initializer.value()));
			returnForStmt();
		}
		else if (std::is_same_v<U, Stmt::VarDecl>) {
			auto type = typeSystem.instantiateType(env, decl.type);
			if (varDef.initializer.has_value()) {
				auto stmts = visitExpr(varDef.initializer.value());
				auto def = IL::Definition(
					createVariable(), IL::Type(type.type->size), getILDefinition(stmts).dest
				);
				env.variables.insert(decl.name, EnvInfo{ std::move(type) }.addDefintion(def.dest));
				returnForStmt(util::combine_vectors(std::move(stmts), IL::makeIL<IL::Definition>(def)));
			}
		}
	}, varDef.decl);
}
void Compiler::visit(Stmt::CountLoop& loop)
{
}
void Compiler::visit(Stmt::Assign& assign)
{
}
void Compiler::visit(Stmt::If& ifStmt)
{
}
void Compiler::visit(Stmt::Module& mod)
{
}

void Compiler::visit(Stmt::Import& imp)
{
}

void Compiler::visit(Stmt::ExprStmt& exprStmt)
{
}

void Compiler::visit(Stmt::Return& stmt)
{
	auto stmts = visitExpr(stmt.expr);
	returnForStmt(util::combine_vectors(std::move(stmts)/*, IL::makeIL<IL::Return>(getILDefinition(stmts).dest)*/));
}

void Compiler::visit(Stmt::Label& label)
{
	auto newLabel = create_label();
	env.labels.insert(label.label, newLabel.name);
	returnForStmt(IL::makeIL<IL::Label>(newLabel));
}

void Compiler::visit(Stmt::Instruction& stmt)
{
	returnForStmt(IL::makeIL<IL::Instruction>(std::move(stmt)));
}

void Compiler::visit(Expr::Binary& expr)
{
	auto variable = create_variable();
	auto lhs = visitExpr(expr.lhs);
	auto rhs = visitExpr(expr.lhs);
	auto& lhs_def = getILDefinition(lhs);
	auto& rhs_def = getILDefinition(rhs);
	assert(lhs_def.type == rhs_def.type);
	auto operation = IL::makeIL<IL::Binary>(variable, lhs_def.type, lhs_def.dest, expr.oper, rhs_def.dest);
	returnForExpr(util::combine_vectors(std::move(lhs), std::move(rhs), std::move(operation)));
}

void Compiler::visit(Expr::Unary& expr)
{
	auto variable = create_variable();
	auto lhs = visitExpr(expr.expr);
	auto lhs_def = getILDefinition(lhs);
	auto operation = IL::makeIL<IL::Unary>(variable, lhs_def.type, lhs_def.dest, expr.oper);
	returnForExpr(util::combine_vectors(std::move(lhs), std::move(operation)));
}

void Compiler::visit(Expr::Parenthesis& expr)
{
}

void Compiler::visit(Expr::Identifier& expr)
{
	
}

void Compiler::visit(Expr::FunctionCall& expr)
{
}

void Compiler::visit(Expr::TemplateCall& expr)
{
}

void Compiler::visit(Expr::Indexing& expr)
{
}

void Compiler::visit(Expr::MemberAccess& expr)
{
	returnForExpr();
}

void Compiler::visit(Expr::Literal& expr)
{
	std::visit([&](auto&& arg) {
		using U = std::remove_cvref_t<decltype(arg)>;
		if constexpr (std::is_same_v<U, std::string>) {
			returnForExpr(IL::makeIL<IL::Definition>(createVariable(), IL::Type(IL::DataType::str, true), arg));
		}
		else if constexpr (std::is_same_v<U, u16>) {
			returnForExpr(IL::makeIL<IL::Definition>(createVariable(), IL::Type(IL::DataType::i16, false), arg));
		}
		else {
			throw;
		}
	}, expr.literal);
}

void Compiler::visit(Expr::CurrentPC& expr)
{
	returnForExpr(IL::makeIL<IL::Definition>(createVariable(), IL::Type(IL::DataType::i16), IL::PC{}));
}

void Compiler::visit(Expr::Register& expr)
{//err
}

void Compiler::visit(Expr::Flag& expr)
{//err
}

