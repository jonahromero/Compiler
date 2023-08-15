#include "TypeSystem.h"
#include "ExprInterpreter.h"
#include "Enviroment.h"
#include <array>

TypeSystem::TypeSystem(Enviroment& env)
	: env(env)
{
	for (auto [name, size] : std::array<std::pair<const char*, size_t>, 5>
	{ { {"u16", 2}, { "i16", 2 }, { "u8", 1 }, { "i8", 1 }, { "bool", 1 }}}) {
		types.emplace_back(std::make_unique<PrimitiveType>(name, size));
	}
}

void TypeSystem::addFunc(Stmt::Function& func)
{
	if (func.isTemplate()) {
		types.push_back(std::make_unique<Template<Stmt::Function>>(std::string{ func.name }, func.deepCopy(), 2));
	}
	else {
		FunctionType newFunc{ std::string{func.name}, 2 };
		newFunc.retType = instantiateType(func.retType).type;
		newFunc.params.reserve(func.params.size());
		for (auto& oldType : func.params) {
			newFunc.params.push_back(instantiateType(oldType.type).type);
		}
		types.push_back(std::make_unique<FunctionType>(std::move(newFunc)));
	}
}

void TypeSystem::addBin(Stmt::Bin& bin)
{
	if (bin.isTemplate()) {
		types.push_back(std::make_unique<Template<Stmt::Bin>>(std::string{ bin.name }, bin.deepCopy(), -1));
	}
	else {
		BinType newType(std::string{ bin.name }, 0);
		for (auto& varDecl : bin.body) {
			auto memType = instantiateType(varDecl.type).type;
			newType.innerTypes.push_back(BinType::Field{ memType, varDecl.name });
			newType.size += memType->size;
		}
		types.emplace_back(std::make_unique<BinType>(newType));
	}
}

void TypeSystem::addAlias(std::string_view name, TypeInstance instance)
{
	aliases.emplace(name, instance.type->name);
}

auto TypeSystem::instantiateType(Expr::UniquePtr& expr) -> TypeSystem::TypeInstance
{
	ExprResult result = ExprInterpreter{ env, *this }.visitPtr(expr);
	if (!std::holds_alternative<TypeSystem::TypeInstance>(result)) {
		throw;
	}
	return std::move(std::get<TypeSystem::TypeInstance>(result));
}

auto TypeSystem::getType(std::string_view name) -> Type const&
{
	if (auto it = aliases.find(name); it != aliases.end()) {
		name = it->second;
	}
	for (auto& type : types) {
		if (type->name == name) {
			return *type;
		}
	}
	assert(false && "Unreachable");
}

auto TypeSystem::isType(std::string_view name) -> bool
{
	if (auto it = aliases.find(name); it != aliases.end()) {
		name = it->second;
	}
	for (auto& type : types) {
		if (type->name == name) {
			return true;
		}
	}
	return false;
}
