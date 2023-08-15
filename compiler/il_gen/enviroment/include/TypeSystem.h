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

	void addBin(Stmt::Bin bin);
	void addBin(std::string name, std::vector<Stmt::VarDecl> decls);
	TypePtr addFunction(std::vector<TypeInstance> paramTypes, TypeInstance returnType);
	
	size_t calculateTypeSize(TypeInstance type) const;
	IL::Type compileType(TypeInstance type);

	Type const& getType(std::string_view name) const;
	bool isType(std::string_view name) const;


private:
	static constexpr size_t POINTER_SIZE = 2;
	Type const* searchTypes(std::string_view name) const;
	bool isPrimitiveType(std::string_view name) const;

	// Naming
	std::string createFunctionName(std::vector<TypeInstance> const& paramTypes, TypeInstance const& returnType) const;

	Enviroment& env;
	std::vector<PrimitiveType> primitiveTypes;
	std::unordered_map<TypePtr, IL::Type> compiledPrimitiveTypes;
	std::vector<std::unique_ptr<Type>> types;
};

