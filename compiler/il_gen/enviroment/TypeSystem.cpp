
#include <array>
#include "TypeSystem.h"
#include "SemanticError.h"
#include "TemplateReplacer.h"
#include "StringUtil.h"

TypeSystem::TypeSystem(Enviroment& env)
	: env(env)
{
	using enum IL::Type;
	// These are all the built in default types
	primitiveTypes.reserve(5);
	for (auto [name, size, compiled] : std::array<std::tuple<const char*, size_t, IL::Type>, 5>
	{ { {"u16", 2, u16}, { "i16", 2, i16}, { "u8", 1, u8 }, { "i8", 1, i8}, { "bool", 1, i1 }}}) {
		primitiveTypes.emplace_back(name, size);
		compiledPrimitiveTypes.emplace(&primitiveTypes.back(), compiled);
	}
}

void TypeSystem::addBin(Stmt::Bin bin)
{
	if (bin.isTemplate()) {
		TemplateBin newType(std::string{ bin.name });
		for (auto& param : bin.templateInfo.params) {
			std::visit([&](auto&& decl) {
				using U = std::remove_cvref_t<decltype(decl)>;
				if constexpr (std::is_same_v<U, Stmt::VarDecl>) {
					TypeInstance declType = env.instantiateType(decl.type);
					if (!isPrimitiveType(declType.type->name)) {
						throw SemanticError(bin.sourcePos, "Only primitive types are allowed as template value parameters");
					}
					newType.templateParams.emplace_back(TemplateBin::TemplateParam{ decl.name, declType });
				}
				else if (std::is_same_v<U, Stmt::TypeDecl>) {
					newType.templateParams.emplace_back(TemplateBin::TemplateParam{ decl.name, TemplateBin::TypeParam{} });
				}
			}, param);
		}
		newType.body = std::move(bin.body);
		types.emplace_back(std::make_unique<TemplateBin>(std::move(newType)));
	}
	else {
		addBin(std::string{ bin.name }, std::move(bin.body));
	}
}

void TypeSystem::addBin(std::string name, std::vector<Stmt::VarDecl> decls)
{
	BinType newType(std::move(name), 0);
	for (auto& varDecl : decls) {
		auto memType = env.instantiateType(varDecl.type);
		newType.members.push_back(BinType::Field{ memType, varDecl.name, newType.size });
		newType.size += memType.type->size;
	}
	types.emplace_back(std::make_unique<BinType>(std::move(newType)));
}

TypePtr TypeSystem::addFunction(std::vector<TypeInstance> paramTypes, TypeInstance returnType)
{
	std::string name = createFunctionName(paramTypes, returnType);
	auto function = std::make_unique<FunctionType>(
		std::move(name), POINTER_SIZE,
		std::move(paramTypes), std::move(returnType)
	);
	types.emplace_back(std::move(function));
	return &types.back();
}

Type const* TypeSystem::searchTypes(std::string_view name) const 
{
	auto primIt = std::find_if(primitiveTypes.begin(), primitiveTypes.end(), [&](auto const& type) { return type.name == name; });
	if (primIt != primitiveTypes.end()) return &(*primIt);
	auto it = std::find_if(types.begin(), types.end(), [&](auto const& type) { return type->name == name; });
	if (it != types.end()) return &(**it);
	return nullptr;
}

bool TypeSystem::isPrimitiveType(std::string_view name) const
{
	auto primIt = std::find_if(primitiveTypes.begin(), primitiveTypes.end(), 
		[&](auto const& type) { return type.name == name; }
	);
	return primIt != primitiveTypes.end();
}

IL::Type TypeSystem::compileType(TypeInstance type)
{
	if (type.isOpt || type.isRef) {
		return IL::Type::u8_ptr;
	}
	else if (auto it = compiledPrimitiveTypes.find(type.type); it != compiledPrimitiveTypes.end())
	{
		return it->second;
	}
	else 
	{
		return IL::Type::u8_ptr;
	}
}

size_t TypeSystem::calculateTypeSize(TypeInstance type) const
{
	if (type.isRef) {
		if (type.isOpt)
			return POINTER_SIZE + 1;
		return POINTER_SIZE;
	}
	else { // Not a reference
		if (type.isMut) return POINTER_SIZE;	// If it's mutable we need to be able to modify it
		size_t totalSize = type.type->size + (type.isOpt ? 1 : 0);
		return totalSize;
	}
}

std::string TypeSystem::createFunctionName(std::vector<TypeInstance> const& paramTypes, TypeInstance const& returnType) const
{
	std::string name = "(";
	for (auto& paramType : paramTypes) {
		name += paramType.type->name;
		name.push_back(',');
	}
	if (name.back() == ',') name.pop_back();
	name += ")->";
	name += returnType.type->name;
	return name;
}

Type const& TypeSystem::getType(std::string_view name) const
{
	return *searchTypes(name);
}

bool TypeSystem::isType(std::string_view name) const
{
	return searchTypes(name) != nullptr;
}
