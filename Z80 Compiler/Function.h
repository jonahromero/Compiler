#pragma once
#include "Type.h"
#include <vector>

struct Function {
	std::string_view ident;
	std::vector<Type> args;
	Type ret;
};