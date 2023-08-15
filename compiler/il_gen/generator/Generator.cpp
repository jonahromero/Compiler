
#include "Generator.h"
#include "Enviroment.h"

namespace gen
{
	Generator::Generator(Enviroment& env)
		: env(env)
	{
	}

	gen::Variable Generator::allocateVariable(IL::Program& instructions, TypeInstance type)
	{
		size_t typeSizeBits = calculateTypeSizeBits(type);
		size_t typeSizeBytes = calculateTypeSizeBytes(type);

		if (type.isRef && !type.isOpt)
		{
			IL::Type impl = getPointerImplementation();
			return gen::Variable
			{
				env.createAnonymousVariable(impl),
				gen::ReferenceType::VALUE,
				type
			};
		}
		else if (typeSizeBits > LARGEST_REGISTER_SIZE_BITS)
		{
			IL::Variable buffer = simpleAllocate(instructions, typeSizeBytes);
			return gen::Variable {
				buffer, gen::ReferenceType::POINTER, type
			};
		}
		else
		{
			bool isUnsigned = isUnsignedType(type);
			IL::Type impl = getValueImplementation(typeSizeBits, isUnsigned);
			return gen::Variable
			{
				env.createAnonymousVariable(impl),
				gen::ReferenceType::VALUE,
				type
			};
		}
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
						!fitsInRegister(ptr.type));
		COMPILER_ASSERT("Allocated variable should be a value type",
						dereferenced.refType == gen::ReferenceType::VALUE);
		COMPILER_ASSERT("An optional reference should never fit in a register", 
						ptr.type.isRef && ptr.type.isOpt ? !fitsInRegister(ptr.type) : true);

		IL::Type ilOutType = env.getILAliasType(dereferenced.ilName);
		if (ptr.refType == gen::ReferenceType::POINTER) 
		{
			if (ptr.type.isOpt)
			{
				ptr.ilName = addToPointer(instructions, ptr.ilName, 1);
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

	gen::Variable Generator::assignVariable(IL::Program instructions, gen::Variable dest, gen::Variable src)
	{
		COMPILER_ASSERT("Cannot assign a variable to another whose size is smaller than it.",
						calculateTypeSizeBits(dest.type) < calculateTypeSizeBits(src.type));

		auto destILType = env.getILAliasType(dest.ilName);
		auto srcILType = env.getILAliasType(src.ilName);
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
				instructions.push_back(IL::makeIL<IL::MemCopy>(dest.ilName, src.ilName, calculateTypeSizeBytes(src.type)));
				break;
			}
			break;
		}
		return dest;
	}
	
	gen::Variable Generator::castVariable(IL::Program& instructions, gen::Variable src, TypeInstance cast)
	{
		if (src.refType == gen::ReferenceType::POINTER)
			return src;
		
		gen::Variable out = allocateVariable(instructions, cast);
		IL::Type castedILType = env.getILAliasType(out.ilName);
		switch (out.refType)
		{
		case gen::ReferenceType::VALUE:
			instructions.push_back(IL::makeIL<IL::Cast>(out.ilName, castedILType, src.ilName));
			break;
		case gen::ReferenceType::POINTER:
			instructions.push_back(IL::makeIL<IL::Store>(out.ilName, src.ilName, env.getILAliasType(src.ilName)));
			break;
		}
		return out;
	}

	gen::Variable Generator::castReferenceType(IL::Program& instructions, gen::Variable var, gen::ReferenceType newRefType)
	{
		if (var.refType == newRefType) 
			return var;

		switch (newRefType) 
		{
		case gen::ReferenceType::VALUE:
			return derefVariable(instructions, var);
		case gen::ReferenceType::POINTER:
			gen::Variable ptr = allocateVariable(getPointerImplementation());
			instructions.push_back(IL::makeIL<IL::AddressOf>(var.ilName, ptr.ilName));
			return ptr;
		}
	}

	IL::Variable Generator::simpleAllocate(IL::Program& instructions, size_t size)
	{
		IL::Variable result = createNewILVariable(getPointerImplementation());
		instructions.push_back(IL::makeIL<IL::Allocate>(result, size));
		return result;
	}

	IL::Variable Generator::simpleDeref(IL::Program& instructions, IL::Type type, IL::Variable ptr)
	{
		COMPILER_ASSERT("Cannot dereference a non-pointer type", env.getILAliasType(ptr) == getPointerImplementation());
		IL::Variable result = createNewILVariable(type);
		instructions.push_back(IL::makeIL<IL::Deref>(result, type, ptr));
		return result;
	}

	IL::Variable Generator::addToPointer(IL::Program& instructions, IL::Variable ptr, int offset)
	{
		IL::Variable lhs = ptr, rhs = createNewILVariable(IL::Type::u16);
		IL::Variable result = createNewILPointer();
		instructions.push_back(IL::makeIL<IL::Assignment>(rhs, IL::Type::u16, offset));
		instructions.push_back(IL::makeIL<IL::Binary>(result, getPointerImplementation(), lhs, Token::Type::PLUS, rhs));
		return result;
	}

	IL::Variable Generator::simpleCast(IL::Program& instructions, IL::Type newType, IL::Variable var)
	{
		if (env.getILAliasType(var) == newType) return var;
		auto castedVariable = env.createAnonymousVariable(newType);
		instructions.emplace_back(IL::makeIL<IL::Cast>(castedVariable, newType, var));
		return castedVariable;
	}

	bool Generator::fitsInRegister(TypeInstance type) const
	{
		return calculateTypeSizeBits(type) <= LARGEST_REGISTER_SIZE_BITS;
	}

	bool Generator::isUnsignedType(TypeInstance type) const
	{
		if (type.isOpt || type.isOpt) return true;
		if (auto primitiveType = type.type->getExactType<PrimitiveType>()) 
		{
			return primitiveType->isUnsigned();
		}
		else {
			return true;
		}
	}
	
	size_t Generator::calculateTypeSizeBytes(TypeInstance type) const
	{
		size_t sizeInBits = calculateTypeSizeBits(type);
		return sizeInBits == 1 ? 1 : sizeInBits / 8;
	}

	size_t Generator::calculateTypeSizeBits(TypeInstance type) const
	{
		if (auto primitiveType = type.type->getExactType<PrimitiveType>()) 
		{
			if (primitiveType->subtype == PrimitiveType::SubType::bool_
				&& !type.isRef && !type.isOpt)
				return 1;
		}
		size_t size = type.isRef ? POINTER_SIZE_BITS : (type.type->size * 8);
		return size + (type.isOpt ? 1 : 0);
	}

	bool Generator::shouldPassInReturnValue(FunctionType const& function) const
	{
		return !fitsInRegister(function.returnType);
	}

	bool Generator::canDereferenceValue(gen::Variable var, TypeInstance type) const
	{
		return (var.refType == gen::ReferenceType::POINTER || type.isRef) 
				&& fitsInRegister(TypeInstance(type.type));
	}

	gen::ReferenceType Generator::getParameterReferenceType(TypeInstance type) const
	{
		// We will optimize the case that an argument is immutable and 
		// fits in a register and simply pass it in by value
		if (!type.isMut && !type.isRef && fitsInRegister(type)) 
		{
			return gen::ReferenceType::VALUE;
		}
		else 
		{
			return gen::ReferenceType::POINTER;
		}
	}

	IL::Type Generator::getValueImplementation(size_t sizeInBits, bool isUnsigned) const
	{
		COMPILER_ASSERT("Cannot create a value implementation larger than register",
						sizeInBits > LARGEST_REGISTER_SIZE_BITS);
		switch (sizeInBits) 
		{
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

	IL::Variable Generator::createNewILPointer()
	{
		return createNewILVariable(getPointerImplementation());
	}

	IL::Variable Generator::createNewILVariable(IL::Type type)
	{
		return env.createAnonymousVariable(type);
	}
}