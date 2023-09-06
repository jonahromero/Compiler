
#pragma once
#include "IL.h"
#include "Types.h"

namespace gen 
{
	enum class ReferenceType
	{
		VALUE, POINTER
	};

	class Variable
	{
	public:
		IL::Variable ilName;
		ReferenceType refType;
		TypeInstance type;
	};
}