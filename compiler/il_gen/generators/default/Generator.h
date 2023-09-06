
#pragma once
#include "SimpleGenerator.h"

namespace gen 
{
	class Generator 
		: protected SimpleGenerator
	{
	protected:
		Generator(Enviroment& env);

		// There are two different variable abstractions made available by the generator.
		// A variable is a conventional variable. It has gaurenteed storage.
		// A non-possessing variable does not have this same gaurentee. 
		// You can reason about it, with the following logic. If you're able to modify 
		// the original storage, this non-possessing will also be able to modify it.
		// However, a non-possessing might copy the value in the case its constant.
		// this means it might not reflect changes done to it through other means.
		// Mainly used to pass parameters. 

		gen::Variable allocateVariable(IL::Program& instructions, TypeInstance type);
		gen::Variable assignVariable(IL::Program& instructions, gen::Variable dest, gen::Variable src);

		gen::Variable allocateNonPossessingVariable(IL::Program& instructions, TypeInstance type);
		gen::Variable assignNonPossessingVariable(IL::Program& instructions, gen::Variable dest, gen::Variable src);

		// The following create new storage for their results
		gen::Variable copyVariable(IL::Program& instructions, gen::Variable var);
		gen::Variable castVariable(IL::Program& instructions, gen::Variable var, TypeInstance cast);

		gen::Variable getPointerTo(IL::Program& instructions, IL::AddressOf::Function function, FunctionType const* funcType);
		gen::Variable getPointerTo(IL::Program& instructions, gen::Variable var);
		gen::Variable getDataTypeAsValue(IL::Program& instructions, gen::Variable var);

		bool shouldPassReturnAsParameter(TypeInstance returnType) const;
		bool canGetAsValue(gen::Variable var) const;

		gen::Variable getOptionalness(IL::Program& instructions, gen::Variable variable);
	private:
		gen::Variable removeOptionalness(IL::Program& instructions, gen::Variable variable);
		gen::Variable assignReference(IL::Program& instructions, gen::Variable lhs, gen::Variable rhs);

		Enviroment& env;
	};
}