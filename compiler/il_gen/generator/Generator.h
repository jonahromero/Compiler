
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

		// complicated helpers that add multiple instructions based on types
		gen::Variable allocateVariable(IL::Program& instructions, TypeInstance type);
		gen::Variable allocateNamedVariable(IL::Program& instructions, std::string_view name, TypeInstance type);
		gen::Variable copyVariable(IL::Program& instructions, gen::Variable var);
		gen::Variable assignVariable(IL::Program& instructions, gen::Variable dest, gen::Variable src);
		gen::Variable castVariable(IL::Program& instructions, gen::Variable var, TypeInstance cast);
		
		gen::Variable derefVariable(IL::Program& instructions, gen::Variable ptr);
		gen::Variable takeAddressOf(IL::Program& instructions, gen::Variable var);
		
		// Dumb helpers that just add instructions
		IL::Variable simpleAllocate(IL::Program& instructions, size_t size);
		IL::Variable simpleDeref(IL::Program& instructions, IL::Type type, IL::Variable ptr);
		IL::Variable simpleCast(IL::Program& instructions, IL::Type type, IL::Variable var);
		IL::Variable addToPointer(IL::Program& instructions, IL::Variable ptr, int offset);
		
		// helpers regarding types
		size_t calculateTypeSizeBytes(TypeInstance type) const;
		size_t calculateTypeSizeBits(TypeInstance type) const;

		bool canDereferenceValue(gen::Variable var) const;

		gen::Variable allocateParameter(IL::Program& instructions, TypeInstance type);
		gen::Variable allocateReturnValue(TypeInstance type);
		gen::Variable bindParameter(IL::Program& instructions, gen::Variable param, gen::Variable arg);

	private:
		static constexpr size_t POINTER_SIZE_BITS = 16, LARGEST_REGISTER_SIZE_BITS = 16;
		Enviroment& env;

		bool shouldPassInReturnValue(TypeInstance returnType) const;
		bool fitsInRegister(TypeInstance type) const;
		bool isUnsignedType(TypeInstance type) const;

		gen::ReferenceType getParameterReferenceType(TypeInstance type) const;

		IL::Type getValueImplementation(TypeInstance type) const;
		IL::Type getPointerImplementation() const;

		IL::Variable createNewILPointer();
		IL::Variable createNewILVariable(IL::Type type);
	};
}