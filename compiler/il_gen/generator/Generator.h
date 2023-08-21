
#pragma once
#include "IL.h"
#include "Types.h"

class Enviroment;

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

	class Generator
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
		gen::Variable createBinding(IL::Program& instructions, gen::Variable other);
		gen::Variable createBindingWithOffset(IL::Program& instructions, gen::Variable other, TypeInstance newType, size_t offset);

		gen::Variable allocateNonPossessingVariable(IL::Program& instructions, TypeInstance type);
		gen::Variable assignNonPossessingVariable(IL::Program& instructions, gen::Variable dest, gen::Variable src);
		gen::Variable assignVariable(IL::Program& instructions, gen::Variable dest, gen::Variable src);

		// The following create new storage for their results
		gen::Variable copyVariable(IL::Program& instructions, gen::Variable var);
		gen::Variable castVariable(IL::Program& instructions, gen::Variable var, TypeInstance cast);
		gen::Variable takeAddressOf(IL::Program& instructions, gen::Variable var);
		gen::Variable derefVariable(IL::Program& instructions, gen::Variable ptr);

	
		bool shouldPassReturnAsParameter(TypeInstance returnType) const;
		bool canDereferenceValue(gen::Variable var) const;

	private:
		Enviroment& env;

		IL::Type getValueImplementation(TypeInstance type) const;
		IL::Type getPointerImplementation() const;

		// These simple functions are helpers that produce common il instruction sequences
		IL::Variable simpleAddToPointer(IL::Program& instructions, IL::Variable ptr, size_t offset);
		IL::Variable simpleAllocate(IL::Program& instructions, size_t size);
		IL::Variable simpleDeref(IL::Program& instructions, IL::Type type, IL::Variable ptr);
		IL::Variable simpleCast(IL::Program& instructions, IL::Type type, IL::Variable var);
		IL::Variable simpleCopy(IL::Program& instructions, IL::Variable var);
		IL::Variable simpleConstant(IL::Program& instructions, size_t constant);

		IL::Variable simpleNewILVariable(IL::Type type);
		IL::Variable simpleNewILPointer();
	};
}