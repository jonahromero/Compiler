
#include <array>
#include "TypeSystem.h"
#include "SemanticError.h"
#include "TemplateReplacer.h"
#include "StringUtil.h"
#include "VariantUtil.h"

TypeSystem::TypeSystem(Enviroment& env)
	: env(env)
{
	using enum PrimitiveType::SubType;
	static constexpr std::array<std::tuple<PrimitiveType::SubType, const char*, size_t>, 6> primitiveTypes = {
		{ u16, "u16", 2 }, { i16, "i16", 2}, { u8, "u8", 1 }, { i8, "i8", 1}, { bool_, "bool", 1 }, {void_, "void", 0}
	};
	for (auto [subtype, name, size] : primitiveTypes)
	{
		addType(PrimitiveType(subtype, name, size));
	}
}

TypePtr TypeSystem::addBin(Stmt::Bin bin)
{
	if (bin.isTemplate()) 
	{
		TemplateBin newType(std::string{ bin.name });
		newType.body = std::move(bin.body);

		for (Stmt::GenericDecl& param : bin.templateInfo.params) 
		{
			newType.templateParams.push_back(compileTemplateDecl(param));
		}
		return addType(std::move(newType));
	}
	else {
		return addBin(std::string{ bin.name }, std::move(bin.body));
	}
}

TypePtr TypeSystem::addBin(std::string name, std::vector<Stmt::VarDecl> decls)
{
	BinType newType(std::move(name), 0);
	for (auto& varDecl : decls) 
	{
		BinType::Field field = compileBinDecl(varDecl, newType.size);
		newType.size += calculateTypeSize(field.type);
		newType.members.push_back(std::move(field));
	}
	return addType(std::move(newType));
}

TypePtr TypeSystem::addFunction(std::vector<TypeInstance> paramTypes, TypeInstance returnType)
{
	std::string name = createFunctionName(paramTypes, returnType);
	return addType(FunctionType{
		std::move(name), POINTER_SIZE,
		std::move(paramTypes), std::move(returnType)
	});
}

TypePtr TypeSystem::addArray(TypeInstance elementType, size_t elements)
{
	std::string name = fmt::format("{}[{}]", ptr->name, std::to_string(elements));
	return addType(ArrayType{ name, elements, elementType, std::vector<size_t>({elements}) });
}

TypePtr TypeSystem::modifyAndAddArray(ArrayType const* arrayType, size_t elements)
{
	ArrayType newType = *arrayType;
	newType.indexing.push_back(elements);
	newType.size *= elements;
	newType.name.append(fmt::format("[{}]", std::to_string(elements)));
	return addType(std::move(newType));
}

TemplateBin::Parameter TypeSystem::compileTemplateDecl(Stmt::GenericDecl const& decl)
{
	std::visit(util::OverloadVariant
	{
		[&](Stmt::VarDecl const& varDecl)
		{
			TypeInstance declType = env.instantiateType(varDecl.type);
			if (!declType.type->getExactType<PrimitiveType>())
			{
				throw SemanticError(varDecl.type->sourcePos, "Only primitive types are allowed as template value parameters");
			}
			return TemplateBin::Parameter{ varDecl.name, declType };
		},
		[&](Stmt::TypeDecl const& typeDecl)
		{
			return TemplateBin::Parameter{ typeDecl.name, TemplateBin::TypeParam{} };
		}
	}, decl);
}

BinType::Field TypeSystem::compileBinDecl(Stmt::VarDecl const& decl, size_t offset)
{
	auto memType = env.instantiateType(varDecl.type);
	return BinType::Field{ memType, varDecl.name, offset };
}

Type const* TypeSystem::searchTypes(std::string_view name) const 
{
	auto it = std::find_if(types.begin(), types.end(), 
	[&](auto const& type) 
	{ 
		return type->name == name; 
	});
	return it != types.end() ? &(**it) : nullptr;
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

TypePtr TypeSystem::getPrimitiveType(PrimitiveType::SubType subtype) const
{
	for (auto& type : types) 
	{
		if (auto primitiveType = type->getExactType<PrimitiveType>())
		{
			if (primitiveType->subtype == subtype)
			{
				return type;
			}
		}
	}
	COMPILER_NOT_REACHABLE;
}

TypePtr TypeSystem::getType(std::string_view name) const
{
	return searchTypes(name);
}

bool TypeSystem::isType(std::string_view name) const
{
	return searchTypes(name) != nullptr;
}

TypePtr TypeSystem::getVoidType() const
{
	return getPrimitiveType(PrimitiveType::SubType::void_);
}