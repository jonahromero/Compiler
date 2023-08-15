#include "TypeSystem.h"
#include "ExprInterpreter.h"
#include "Enviroment.h"
#include <array>

TypeSystem::TypeSystem()
{
	for (auto [name, size] : std::array<std::pair<const char*, size_t>, 5>
	{ { {"u16", 2}, { "i16", 2 }, { "u8", 1 }, { "i8", 1 }, { "bool", 1 }}}) {
		types.emplace_back(std::make_unique<PrimitiveType>(name, size));
	}
}

void TypeSystem::addFunc(Stmt::Function const& func)
{
	if (func.isTemplate()) {
	}
	else {
		/*
		FunctionType newFunc;
		newFunc.name = func.name;
		newFunc.retType = func.retType;
		newFunc.params.reserve(func.params.size());
		for (auto oldType : func.params) {
			newFunc.params.push_back(compileType(oldType));
		}
		types.emplace_back(std::make_unique<BinType>(newType));
		*/
	}
}

void TypeSystem::addBin(Stmt::Bin const& bin)
{
	if (bin.isTemplate()) {
	}
	else {
		/*BinType newType;
		newType.name = bin.name;
		newType.size = 0;
		for (auto& varDecl : bin.body) {
			auto memType = compileType(varDecl.type);
			newType.innerTypes.push_back(memType);
			newType.size += memType;
		}
		types.emplace_back(std::make_unique<BinType>(newType));
		*/
	}
}

void TypeSystem::addAlias(std::string_view name, TypeInstance instance)
{
}

auto TypeSystem::instantiateType(Enviroment& env, Expr::UniquePtr& expr) -> TypeSystem::TypeInstance
{
	ExprResult result = ExprInterpreter{ env, *this }.visitPtr(expr);
	if (!std::holds_alternative<TypeSystem::TypeInstance>(result)) {
		throw;
	}
	return std::move(std::get<TypeSystem::TypeInstance>(result));
}

auto TypeSystem::getType(std::string_view name) -> Type const&
{
	// // O: insert return statement here
}

auto TypeSystem::isType(std::string_view name) -> bool
{
	return false;
}
