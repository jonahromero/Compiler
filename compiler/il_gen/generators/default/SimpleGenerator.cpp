#include "SimpleGenerator.h"

#include "SimpleGenerator.h"
#include "TargetInfo.h"

namespace gen 
{
	SimpleGenerator::SimpleGenerator(Enviroment& env)
		: env(env)
	{
	}

	IL::Variable SimpleGenerator::simpleAllocate(IL::Program& instructions, size_t size)
	{
		IL::Variable result = simpleNewILPointer();
		instructions.push_back(IL::makeIL<IL::Allocate>(result, size));
		return result;
	}

	IL::Variable SimpleGenerator::simpleDeref(IL::Program& instructions, IL::Type type, IL::Variable ptr)
	{
		COMPILER_ASSERT("Cannot dereference a non-pointer type", env.getILVariableType(ptr) == getPointerImplementation());
		IL::Variable result = simpleNewILVariable(type);
		instructions.push_back(IL::makeIL<IL::Deref>(result, type, ptr));
		return result;
	}

	IL::Variable SimpleGenerator::simpleAddToPointer(IL::Program& instructions, IL::Variable ptr, IL::Variable offset)
	{
		IL::Variable result = simpleNewILPointer();
		instructions.push_back(IL::makeIL<IL::Binary>(result, getPointerImplementation(), ptr, Token::Type::PLUS, offset));
		return result;
	}

	IL::Variable SimpleGenerator::simpleCast(IL::Program& instructions, IL::Type newType, IL::Variable var)
	{
		auto castedVariable = env.createAnonymousVariable(newType);
		instructions.emplace_back(IL::makeIL<IL::Cast>(castedVariable, newType, var));
		return castedVariable;
	}

	IL::Variable SimpleGenerator::simpleConstant(IL::Program& instructions, size_t constant)
	{
		IL::Variable result = simpleNewILVariable(IL::Type::u16);
		instructions.push_back(IL::makeIL<IL::Assignment>(result, IL::Type::u16, int(constant)));
		return result;
	}

	IL::Variable SimpleGenerator::simpleCopy(IL::Program& instructions, IL::Variable var)
	{
		IL::Type type = env.getILVariableType(var);
		IL::Variable copied = env.createAnonymousVariable(type);
		instructions.push_back(IL::makeIL<IL::Assignment>(copied, type, var));
		return copied;
	}

	IL::Type SimpleGenerator::getValueImplementation(TypeInstance type) const
	{
		if (type.isRef) return getPointerImplementation();
		bool isUnsigned = TargetInfo::isUnsignedType(type);
		size_t sizeInBits = TargetInfo::calculateTypeSizeBits(type);
		COMPILER_ASSERT("Cannot create a value implementation larger than register",
			TargetInfo::fitsInRegister(type));
		switch (sizeInBits)
		{
		case 0: return IL::Type::void_;
		case 1: return IL::Type::i1;
		case 8: return isUnsigned ? IL::Type::u8 : IL::Type::i8;
		case 16: return isUnsigned ? IL::Type::u16 : IL::Type::i16;
		default: COMPILER_NOT_REACHABLE;
		}
	}

	IL::Type SimpleGenerator::getIndexImplementation() const
	{
		return IL::Type::u16;
	}

	IL::Type SimpleGenerator::getPointerImplementation() const
	{
		return IL::Type::u8_ptr;
	}

	IL::Variable SimpleGenerator::simpleNewILPointer()
	{
		return simpleNewILVariable(getPointerImplementation());
	}

	IL::Variable SimpleGenerator::simpleNewILVariable(IL::Type type)
	{
		return env.createAnonymousVariable(type);
	}
}