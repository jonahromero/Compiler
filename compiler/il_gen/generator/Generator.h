
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
		gen::Variable copyVariable(IL::Program& instructions, gen::Variable var);
		gen::Variable derefVariable(IL::Program& instructions, gen::Variable ptr);
		gen::Variable assignVariable(IL::Program instructions, gen::Variable dest, gen::Variable src);
		gen::Variable castVariable(IL::Program& instructions, gen::Variable var, TypeInstance cast);
		gen::Variable castReferenceType(IL::Program& instructions, gen::Variable var, gen::ReferenceType newRefType);
		
		// Dumb helpers that just add instructions
		IL::Variable simpleAllocate(IL::Program& instructions, size_t size);
		IL::Variable simpleDeref(IL::Program& instructions, IL::Type type, IL::Variable ptr);
		IL::Variable simpleCast(IL::Program& instructions, IL::Type type, IL::Variable var);
		IL::Variable addToPointer(IL::Program& instructions, IL::Variable ptr, int offset);
		
		// helpers regarding types
		size_t calculateTypeSizeBytes(TypeInstance type) const;
		size_t calculateTypeSizeBits(TypeInstance type) const;

		bool shouldPassInReturnValue(FunctionType const& function) const;
		bool canDereferenceValue(gen::Variable var, TypeInstance type) const;

		gen::ReferenceType getParameterReferenceType(TypeInstance type) const;

	private:
		static constexpr size_t POINTER_SIZE_BITS = 16, LARGEST_REGISTER_SIZE_BITS = 16;
		Enviroment& env;

		bool fitsInRegister(TypeInstance type) const;
		bool isUnsignedType(TypeInstance type) const;

		IL::Type getValueImplementation(size_t sizeInBits, bool isUnsigned) const;
		IL::Type getPointerImplementation() const;

		IL::Variable createNewILPointer();
		IL::Variable createNewILVariable(IL::Type type);
	};
}