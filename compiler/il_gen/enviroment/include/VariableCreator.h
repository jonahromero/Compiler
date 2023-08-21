
#pragma once
#include <IL.h>

class VariableCreator 
{
public:
	auto createVariable() -> IL::Variable {
		return IL::Variable(variable++, false);
	}
private:
	size_t variable = 0;
};