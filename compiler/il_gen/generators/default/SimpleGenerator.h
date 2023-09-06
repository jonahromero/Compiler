
#pragma once
#include "IL.h"
#include "Types.h"
#include "Enviroment.h"

namespace gen
{
	class SimpleGenerator
	{
	protected:
		SimpleGenerator(Enviroment& env);

		IL::Type getValueImplementation(TypeInstance type) const;
		IL::Type getIndexImplementation() const;
		IL::Type getPointerImplementation() const;

		// These simple functions are helpers that produce common il instruction sequences
		IL::Variable simpleAddToPointer(IL::Program& instructions, IL::Variable ptr, IL::Variable offset);
		IL::Variable simpleAllocate(IL::Program& instructions, size_t size);
		IL::Variable simpleDeref(IL::Program& instructions, IL::Type type, IL::Variable ptr);
		IL::Variable simpleCast(IL::Program& instructions, IL::Type type, IL::Variable var);
		IL::Variable simpleCopy(IL::Program& instructions, IL::Variable var);
		IL::Variable simpleConstant(IL::Program& instructions, size_t constant);

		IL::Variable simpleNewILVariable(IL::Type type);
		IL::Variable simpleNewILPointer();

	private:
		Enviroment& env;
	};
}

