
#pragma once
#include "Generator.h"

namespace gen 
{
	class GeneratorToolKit
		: protected Generator
	{
	protected:
		GeneratorToolKit(Enviroment& env);

		gen::Variable createBinding(IL::Program& instructions, gen::Variable other);
		gen::Variable createBindingWithOffset(IL::Program& instructions, gen::Variable other, TypeInstance newType, gen::Variable offset);
		gen::Variable allocateConstant(IL::Program& instructions, size_t constant, PrimitiveType::SubType type);
	private:
		Enviroment& env;
	};
}

