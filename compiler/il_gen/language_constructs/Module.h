#pragma once
#include "Function.h"
#include "Bin.h"

class Module
{
	std::string_view name;
	std::vector<Function> functions;
	std::vector<Type> types;
};

