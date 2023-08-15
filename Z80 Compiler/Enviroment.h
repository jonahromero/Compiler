#pragma once
#include <vector>
#include "ArrayMap.h"
#include "TypeSystem.h"
#include "IL.h"

struct EnvInfo {
	EnvInfo(TypeSystem::TypeInstance type) 
		: type(std::move(type)) {}
	EnvInfo&& addDefintion(IL::Variable variable) && {
		currentILVariables.push_back(std::move(variable));
		return std::move(*this);
	}

	TypeSystem::TypeInstance type;
	std::vector<IL::Variable> currentILVariables;
};

class Enviroment 
{
public:
	ArrayMap<std::string_view, EnvInfo> variables;
	ArrayMap<std::string_view, size_t> labels;
};