
#include "Generator.h"
#include "Enviroment.h"
#include "TargetInfo.h"

namespace gen
{
	Generator::Generator(Enviroment& env)
		: env(env)
	{
	}

	gen::Variable Generator::allocateVariable(IL::Program& instructions, TypeInstance type)
	{
		if (!TargetInfo::fitsInRegister(type))
		{
			IL::Variable buffer = simpleAllocate(instructions, TargetInfo::calculateTypeSizeBytes(type));
			return gen::Variable {
				buffer, gen::ReferenceType::POINTER, type
			};
		}
		else
		{
			IL::Type impl = type.isRef ? 
				getPointerImplementation() :
				getValueImplementation(type);
			
			return gen::Variable
			{
				env.createAnonymousVariable(impl),
				gen::ReferenceType::VALUE,
				type
			};
		}
	}

	gen::Variable Generator::allocateNonPossessingVariable(IL::Program& instructions, TypeInstance type)
	{
		return gen::Variable {
			simpleNewILPointer(), gen::ReferenceType::POINTER, type
		};
	}

	gen::Variable Generator::assignNonPossessingVariable(IL::Program& instructions, gen::Variable dest, gen::Variable src)
	{
		if (src.refType == gen::ReferenceType::VALUE) {
			src = takeAddressOf(instructions, src);
		}
		IL::Type destILType = env.getILVariableType(dest.ilName);
		instructions.push_back(IL::makeIL<IL::Assignment>(dest.ilName, destILType, src.ilName));
		return dest;
	}

	gen::Variable Generator::createBinding(IL::Program& instructions, gen::Variable other)
	{
		gen::Variable result = allocateNonPossessingVariable(instructions, other.type);
		assignNonPossessingVariable(instructions, result, other);
		return result;
	}

	gen::Variable Generator::createBindingWithOffset(IL::Program& instructions, gen::Variable target, TypeInstance newType, size_t offset)
	{
		target = createBinding(instructions, target);
		target.ilName = simpleAddToPointer(instructions, target.ilName, offset);
		target.type = newType;
		return target;
	}

	gen::Variable Generator::copyVariable(IL::Program& instructions, gen::Variable src)
	{
		auto copied = allocateVariable(instructions, src.type);
		return assignVariable(instructions, copied, src);
	}
	
	gen::Variable Generator::derefVariable(
		IL::Program& instructions, 
		gen::Variable ptr)
	{
		gen::Variable dereferenced = allocateVariable(instructions, ptr.type);

		COMPILER_ASSERT("Cannot dereference a type that doesnt fit in a register", 
						!TargetInfo::fitsInRegister(ptr.type));
		COMPILER_ASSERT("Allocated variable should be a value type",
						dereferenced.refType == gen::ReferenceType::VALUE);
		COMPILER_ASSERT("An optional reference should never fit in a register", 
						ptr.type.isRef && ptr.type.isOpt ? !TargetInfo::fitsInRegister(ptr.type) : true);

		IL::Type ilOutType = env.getILVariableType(dereferenced.ilName);
		if (ptr.refType == gen::ReferenceType::POINTER)
		{
			if (ptr.type.isOpt)
			{
				ptr.ilName = simpleAddToPointer(instructions, ptr.ilName, 1);
				ptr.type.isOpt = false;
			}
			if (ptr.type.isRef)
			{
				ptr.ilName = simpleDeref(instructions, getPointerImplementation(), ptr.ilName);
				ptr.refType = gen::ReferenceType::VALUE;
			}
		}
		
		COMPILER_ASSERT("Cannot dereference a maybe passed by value", !ptr.type.isOpt);
		COMPILER_ASSERT("If a value is passed, a reference is expected", ptr.type.isRef);
		COMPILER_ASSERT("Can only dereference a reference by value", ptr.refType == gen::ReferenceType::VALUE);
		
		ptr.type.isRef = false;
		return gen::Variable{
			simpleDeref(instructions, ilOutType, dereferenced.ilName),
			gen::ReferenceType::VALUE,
			ptr.type
		};
	}

	gen::Variable Generator::assignVariable(IL::Program& instructions, gen::Variable dest, gen::Variable src)
	{
		COMPILER_ASSERT("Cannot assign a variable to another whose size is smaller than it.",
						TargetInfo::calculateTypeSizeBits(dest.type)
						< TargetInfo::calculateTypeSizeBits(src.type));

		auto destILType = env.getILVariableType(dest.ilName);
		auto srcILType = env.getILVariableType(src.ilName);
		switch (src.refType)
		{
		case gen::ReferenceType::VALUE:
			switch (dest.refType)
			{
			case gen::ReferenceType::VALUE:
				instructions.push_back(IL::makeIL<IL::Assignment>(dest.ilName, destILType, src.ilName));
				break;
			case gen::ReferenceType::POINTER:
				instructions.push_back(IL::makeIL<IL::Store>(dest.ilName, src.ilName, srcILType));
				break;
			}
			break;
		case gen::ReferenceType::POINTER:
			switch (dest.refType)
			{
			case gen::ReferenceType::VALUE:
				instructions.push_back(IL::makeIL<IL::Deref>(dest.ilName, destILType, src.ilName));
				break;
			case gen::ReferenceType::POINTER:
				instructions.push_back(IL::makeIL<IL::MemCopy>(dest.ilName, src.ilName, TargetInfo::calculateTypeSizeBytes(src.type)));
				break;
			}
			break;
		}
		return dest;
	}
	
	gen::Variable Generator::castVariable(IL::Program& instructions, gen::Variable src, TypeInstance cast)
	{
		if (src.refType == gen::ReferenceType::POINTER) {
			gen::Variable copied = copyVariable(instructions, src);
			COMPILER_ASSERT("Copied variable should also be a pointer type", 
							copied.refType == gen::ReferenceType::POINTER);
			copied.type = cast;
			return copied;
		}
		
		gen::Variable out = allocateVariable(instructions, cast);
		IL::Type castedILType = env.getILVariableType(out.ilName);
		switch (out.refType)
		{
		case gen::ReferenceType::VALUE:
			instructions.push_back(IL::makeIL<IL::Cast>(out.ilName, castedILType, src.ilName));
			break;
		case gen::ReferenceType::POINTER:
			instructions.push_back(IL::makeIL<IL::Store>(out.ilName, src.ilName, env.getILVariableType(src.ilName)));
			break;
		}
		return out;
	}

	gen::Variable Generator::takeAddressOf(IL::Program& instructions, gen::Variable var)
	{
		COMPILER_ASSERT("Cannot go past two levels of indirection", 
						var.type.isMut && var.refType == gen::ReferenceType::POINTER);
		COMPILER_ASSERT("Cannot create a pointer of a pointer", 
						var.refType == gen::ReferenceType::POINTER);
		
		IL::Variable ptr = simpleNewILPointer();
		instructions.push_back(IL::makeIL<IL::AddressOf>(ptr, var.ilName));
		return gen::Variable {
			ptr, gen::ReferenceType::POINTER, var.type
		};
	}

	IL::Variable Generator::simpleAllocate(IL::Program& instructions, size_t size)
	{
		IL::Variable result = simpleNewILPointer();
		instructions.push_back(IL::makeIL<IL::Allocate>(result, size));
		return result;
	}

	IL::Variable Generator::simpleDeref(IL::Program& instructions, IL::Type type, IL::Variable ptr)
	{
		COMPILER_ASSERT("Cannot dereference a non-pointer type", env.getILVariableType(ptr) == getPointerImplementation());
		IL::Variable result = simpleNewILVariable(type);
		instructions.push_back(IL::makeIL<IL::Deref>(result, type, ptr));
		return result;
	}

	IL::Variable Generator::simpleAddToPointer(IL::Program& instructions, IL::Variable ptr, size_t offset)
	{
		IL::Variable lhs = ptr, rhs = simpleConstant(instructions, offset);
		IL::Variable result = simpleNewILPointer();
		instructions.push_back(IL::makeIL<IL::Binary>(result, getPointerImplementation(), lhs, Token::Type::PLUS, rhs));
		return result;
	}

	IL::Variable Generator::simpleCast(IL::Program& instructions, IL::Type newType, IL::Variable var)
	{
		if (env.getILVariableType(var) == newType) return var;
		auto castedVariable = env.createAnonymousVariable(newType);
		instructions.emplace_back(IL::makeIL<IL::Cast>(castedVariable, newType, var));
		return castedVariable;
	}

	IL::Variable Generator::simpleConstant(IL::Program& instructions, size_t constant)
	{
		IL::Variable result = simpleNewILVariable(IL::Type::u16);
		instructions.push_back(IL::makeIL<IL::Assignment>(result, IL::Type::u16, int(constant)));
		return result;
	}

	IL::Variable Generator::simpleCopy(IL::Program& instructions, IL::Variable var)
	{
		IL::Type type = env.getILVariableType(var);
		IL::Variable copied = env.createAnonymousVariable(type);
		instructions.push_back(IL::makeIL<IL::Assignment>(copied, type, var));
		return copied;
	}

	bool Generator::shouldPassReturnAsParameter(TypeInstance returnType) const
	{
		return !TargetInfo::fitsInRegister(returnType);
	}

	bool Generator::canDereferenceValue(gen::Variable var) const
	{
		return (var.refType == gen::ReferenceType::POINTER || var.type.isRef) 
				&& TargetInfo::fitsInRegister(TypeInstance(var.type.type));
	}

	IL::Type Generator::getValueImplementation(TypeInstance type) const
	{
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

	IL::Type Generator::getPointerImplementation() const
	{
		return IL::Type::u8_ptr;
	}

	IL::Variable Generator::simpleNewILPointer()
	{
		return simpleNewILVariable(getPointerImplementation());
	}

	IL::Variable Generator::simpleNewILVariable(IL::Type type)
	{
		return env.createAnonymousVariable(type);
	}
}