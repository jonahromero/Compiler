#include "GeneratorToolKit.h"
#include "TargetInfo.h"

namespace gen
{
	GeneratorToolKit::GeneratorToolKit(Enviroment& env)
		: Generator(env), env(env)
	{
	}

	gen::Variable GeneratorToolKit::allocateConstant(IL::Program& instructions, size_t constant, PrimitiveType::SubType type)
	{
		TypeInstance asTypeInstance = env.types.getPrimitiveType(type);
		COMPILER_DEBUG
		{
			auto bits = TargetInfo::calculateTypeSizeBits(asTypeInstance);
			size_t mask = (1 << bits) - 1;
			COMPILER_ASSERT("Constant must fit within subtype", (constant & mask) == constant);
		}
		IL::Type storageType = getValueImplementation(asTypeInstance);
		IL::Variable storage = env.createAnonymousVariable(storageType);

		instructions.push_back(IL::makeIL<IL::Assignment>(
			storage, storageType, int(constant)
		));
		return gen::Variable {
			storage, gen::ReferenceType::VALUE, asTypeInstance
		};
	}

	gen::Variable GeneratorToolKit::createBinding(IL::Program& instructions, gen::Variable other)
	{
		gen::Variable result = allocateNonPossessingVariable(instructions, other.type);
		assignNonPossessingVariable(instructions, result, other);
		return result;
	}

	gen::Variable GeneratorToolKit::createBindingWithOffset(IL::Program& instructions, gen::Variable target, TypeInstance newType, gen::Variable offset)
	{
		offset = getDataTypeAsValue(instructions, offset);
		target = createBinding(instructions, target);
		COMPILER_ASSERT("Assumes non possessing implementation uses gen::ReferenceType::POINTER in order to "
						"add an offset to the pointer, because otherwise whether we should add the offset to"
						"the actual reference type or the pointer is ambiguious", target.refType == gen::ReferenceType::POINTER);
		IL::Variable result = env.createAnonymousVariable(getPointerImplementation());
		instructions.push_back(IL::makeIL<IL::Binary>(
			result, getPointerImplementation(),
			target.ilName, Token::Type::PLUS, offset.ilName
		));
		return gen::Variable{
			result, 
			gen::ReferenceType::POINTER,
			newType
		};
	}


}