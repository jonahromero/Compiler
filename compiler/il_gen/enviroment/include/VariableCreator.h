
#pragma once
#include <IL.h>

class VariableCreator 
{
public:
	auto createVariable() -> IL::Variable {
		return IL::Variable(variable++, false);
	}
	auto createLabel()->IL::Label { return IL::Label(label++); }
private:
	size_t variable = 0, label = 0;
};