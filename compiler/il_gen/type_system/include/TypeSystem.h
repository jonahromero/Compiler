#pragma once
#include <array>
#include <string>
#include <unordered_map>
#include "Stmt.h"
#include "Expr.h"
#include "Types.h"
#include "ScopedContainer.h"
#include "IL.h"

class TypeSystem
{
public:
	TypeSystem();

	void newScope();
	void destroyScope();

	TypePtr addBin(Stmt::Bin bin);
	TypePtr addBin(std::string name, std::vector<Stmt::VarDecl> decls);
	TypePtr addFunction(std::vector<TypeInstance> paramTypes, TypeInstance returnType);
	TypePtr addArray(TypeInstance elementType, size_t elements);
	TypePtr modifyAndAddArray(ListType const* arrayType, size_t elements);
	void addAlias(std::string_view name, TypeInstance actualType);

	TypeInstance instantiateType(Expr::UniquePtr const& expr);
	
	TypeInstance getTypeAlias(std::string_view name) const;
	bool isTypeAlias(std::string_view name) const;
	Type const* getType(std::string_view name) const;
	bool isType(std::string_view name) const;
	
	PrimitiveType const* getPrimitiveType(PrimitiveType::SubType subtype) const;
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

	std::vector<std::unique_ptr<Type>> types;
	ScopedContainer<std::unordered_map<std::string_view, TypeInstance>> aliases;
};

