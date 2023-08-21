#pragma once
#include <array>
#include <string>
#include <unordered_map>
#include "Stmt.h"
#include "Expr.h"
#include "Types.h"
#include "IL.h"

class Enviroment;

class TypeSystem
{
public:
	TypeSystem(Enviroment& env);

	TypePtr addBin(Stmt::Bin bin);
	TypePtr addBin(std::string name, std::vector<Stmt::VarDecl> decls);
	TypePtr addFunction(std::vector<TypeInstance> paramTypes, TypeInstance returnType);
	TypePtr addArray(TypeInstance elementType, size_t elements);
	TypePtr modifyAndAddArray(ArrayType const* arrayType, size_t elements);

	PrimitiveType const* getPrimitiveType(PrimitiveType::SubType subtype) const;
	
	Type const* getType(std::string_view name) const;
	bool isType(std::string_view name) const;
	
	TypePtr getVoidType() const;

private:
	Type const* searchTypes(std::string_view name) const;

	template<typename T>
	TypePtr addType(T&& type) 
	{
		using DerivedType = std::remove_cvref_t<T>;
		static_assert(std::is_base_of_v<Type, DerivedType>, "Must add type that derives from Type");
		types.emplace_back(std::make_unique<DerivedType>(std::forward<T>(type)));
		return types.back().get();
	}

	TemplateBin::Parameter compileTemplateDecl(Stmt::GenericDecl const& decl);
	BinType::Field compileBinDecl(Stmt::VarDecl const& decl, size_t offset);
	// Naming
	std::string createFunctionName(std::vector<TypeInstance> const& paramTypes, TypeInstance const& returnType) const;

	Enviroment& env;
	std::vector<std::unique_ptr<Type>> types;
};

