#pragma once
#include <vector>
#include <unordered_map>
#include "StringUtil.h"
#include "ArrayMap.h"
#include "TypeSystem.h"
#include "VariableCreator.h"
#include "IL.h"

class Enviroment 
{
public:
	Enviroment();

	void startScope();
	void endScope();

	// variables
	IL::Variable createVariable(std::string_view name, TypeInstance type);
	IL::Variable createAnonymousVariable(IL::Type ilType);
	IL::Variable const& getVariableILAlias(std::string_view name) const;
	IL::Type getILAliasType(IL::Variable variable) const;
	bool isValidVariable(std::string_view name) const;
	
	// Variable types
	TypeInstance const& getVariableType(std::string_view name) const;
	TypeInstance instantiateType(Expr::UniquePtr const& expr);

	// Type aliases
	bool isTypeAlias(std::string_view name) const;
	void addTypeAlias(std::string_view name, TypeInstance instance);
	TypeInstance const& getTypeAlias(std::string_view name) const;

	// Type system
	TypeSystem types;

private:
	struct VarInfo
	{
		TypeInstance type;
		IL::Variable ilAlias;
	};

	VarInfo const* searchVariables(std::string_view name) const;
	TypeInstance const* searchTypeAliases(std::string_view name) const;

	VariableCreator variableCreator;

	template<typename T>
	using ScopeContainer = std::vector<T>;
	ScopeContainer<std::unordered_map<std::string_view, VarInfo>> variables;
	ScopeContainer<std::unordered_map<std::string_view, TypeInstance>> typeAliases;
	ScopeContainer<std::unordered_map<IL::Variable, IL::Type>> ilAliasTypes;

	//std::unordered_map<std::string, FunctionInfo, util::StringHash, std::equal_to<>> functions;
};