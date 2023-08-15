#pragma once
#include <vector>
#include <unordered_map>
#include "ArrayMap.h"
#include "TypeSystem.h"
#include "IL.h"

struct EnvInfo 
{
	EnvInfo(TypeSystem::TypeInstance type, IL::Variable var)
		: currentILVar(var), type(std::move(type)) {}

	TypeSystem::TypeInstance type;
	IL::Variable currentILVar;
};

class Enviroment 
{
public:
	std::unordered_map<std::string_view, EnvInfo> variables;
	std::unordered_map<std::string_view, size_t> labels;
};