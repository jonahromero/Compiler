#include "Generator.h"
#include "Generator.h"
#include "Generator.h"
#include "Generator.h"
#include "Generator.h"
#include "Generator.h"
#include "Generator.h"
#include "Generator.h"

#include "Generator.h"
#include "Enviroment.h"
#include "TargetInfo.h"

namespace gen
{
	Generator::Generator(Enviroment& env)
		: SimpleGenerator(env), env(env)
	{
	}

	gen::Variable Generator::allocateVariable(IL::Program& instructions, TypeInstance type)
	{
		if (!TargetInfo::fitsInRegister(type))
		{
			COMPILER_ASSERT("Reference should always fit in value", !type.isRef);
			IL::Variable buffer = simpleAllocate(instructions, TargetInfo::calculateTypeSizeBytes(type));
			return gen::Variable {
				buffer, gen::ReferenceType::POINTER, type
			};
		}
		else
		{
			IL::Type impl =	getValueImplementation(type);
			IL::Variable result = env.createAnonymousVariable(impl);
			instructions.push_back(IL::makeIL<IL::Assignment>(result, impl, 0));
			return gen::Variable
			{
				result, gen::ReferenceType::VALUE, type
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
		COMPILER_ASSERT("destination must be non-possessing variable",
						dest.refType == gen::ReferenceType::POINTER 
						&& env.getILVariableType(dest.ilName) == getPointerImplementation());

		src = getPointerTo(instructions, src);
		instructions.push_back(IL::makeIL<IL::Assignment>(dest.ilName, getPointerImplementation(), src.ilName));
		return dest;
	}

	gen::Variable Generator::copyVariable(IL::Program& instructions, gen::Variable src)
	{
		auto copied = allocateVariable(instructions, src.type);
		return assignVariable(instructions, copied, src);
	}

	gen::Variable Generator::getDataTypeAsValue(IL::Program& instructions, gen::Variable var)
	{
		COMPILER_ASSERT("Must fit in value type", TargetInfo::fitsInRegister(var.type));
		if (var.type.isOpt)
			var = removeOptionalness(instructions, var);
		// we do this in order to make sure the behavior of getAsValue is consistently 
		// a copy, since we need to copy values if ptr is gen::ReferenceType::POINTER instead
		if (var.refType == gen::ReferenceType::VALUE && !var.type.isRef)
		{
			return gen::Variable{
				simpleCopy(instructions, var.ilName),
					gen::ReferenceType::VALUE,
					var.type
			};
		}
		if (var.type.isRef)
		{
			if (var.refType == gen::ReferenceType::POINTER)
			{
				var.ilName = simpleDeref(instructions, getPointerImplementation(), var.ilName);
				var.refType = gen::ReferenceType::VALUE;
			}
			var.type.isRef = false;
			IL::Type type = getValueImplementation(var.type);
			IL::Variable storage = simpleNewILVariable(type);
			instructions.push_back(IL::makeIL<IL::Deref>(storage, type, var.ilName));

			return gen::Variable{
				storage,
				gen::ReferenceType::VALUE,
				var.type
			};
		}
		else {
			IL::Type type = getValueImplementation(var.type);
			return gen::Variable {
				simpleDeref(instructions, type, var.ilName),
				gen::ReferenceType::VALUE,
				var.type
			};
		}
	}

	gen::Variable Generator::assignVariable(IL::Program& instructions, gen::Variable dest, gen::Variable src)
	{
		if (dest.type.isRef && !src.type.isRef) {
			return assignReference(instructions, dest, src);
		}
		if (dest.type != src.type) {
			src = castVariable(instructions, src, dest.type);
		}
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
		if (src.refType == gen::ReferenceType::POINTER)
		{
			size_t bytes = std::min(TargetInfo::calculateTypeSizeBytes(cast), 
									TargetInfo::calculateTypeSizeBytes(src.type));
			IL::Variable buffer = simpleAllocate(instructions, bytes);
			instructions.push_back(IL::makeIL<IL::MemCopy>(buffer, src.ilName, bytes));
			return gen::Variable{
				buffer, gen::ReferenceType::POINTER, cast
			};
		}
		if (src.type.isRef)
		{
			return gen::Variable{
				simpleCopy(instructions, src.ilName),
				gen::ReferenceType::VALUE,
				cast
			};
		}
		// We catch the cases that the variable is a reference of pointer of some kind.
		// Otherwise we allocate a new space for the casted type, and assign it.
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

	gen::Variable Generator::getPointerTo(IL::Program& instructions, gen::Variable var)
	{
		if (var.refType == gen::ReferenceType::POINTER) {
			return gen::Variable {
				simpleCopy(instructions, var.ilName),
				gen::ReferenceType::POINTER,
				var.type
			};
		}
		IL::Variable ptr = simpleNewILPointer();
		instructions.push_back(IL::makeIL<IL::AddressOf>(ptr, var.ilName));
		return gen::Variable {
			ptr, gen::ReferenceType::POINTER, var.type
		};
	}

	gen::Variable Generator::getPointerTo(IL::Program& instructions, IL::AddressOf::Function function, FunctionType const* funcType)
	{
		IL::Variable ptr = simpleNewILPointer();
		instructions.push_back(IL::makeIL<IL::AddressOf>(ptr, function));
		return gen::Variable {
			ptr, gen::ReferenceType::POINTER, TypeInstance(funcType)
		};
	}

	bool Generator::shouldPassReturnAsParameter(TypeInstance returnType) const
	{
		return !TargetInfo::fitsInRegister(returnType);
	}

	bool Generator::canGetAsValue(gen::Variable var) const
	{
		return TargetInfo::fitsInRegister(var.type);
	}

	// PRIVATE HELPERS

	gen::Variable Generator::getOptionalness(IL::Program& instructions, gen::Variable var)
	{
		COMPILER_ASSERT("Variable must be optional", var.type.isOpt);
		TypeInstance boolInstance = env.types.getPrimitiveType(PrimitiveType::SubType::bool_);
		if (var.refType == gen::ReferenceType::POINTER)
		{
			IL::Variable byte = simpleDeref(instructions, IL::Type::u8, var.ilName);
			IL::Variable flag = simpleCast(instructions, IL::Type::i1, byte);
			return gen::Variable {
				byte, gen::ReferenceType::VALUE, boolInstance
			};
		}
		else
		{
			IL::Variable isValidFlag = env.createAnonymousVariable(IL::Type::i1);
			size_t bit = 0;
			if (TargetInfo::isLittleEndian()) {
			
			}
			else {
				
			}
			instructions.push_back(IL::makeIL<IL::TestBit>(isValidFlag, var.ilName, bit));
			return gen::Variable {
				isValidFlag, gen::ReferenceType::VALUE, boolInstance
			};
		}
	}

	gen::Variable gen::Generator::removeOptionalness(IL::Program& instructions, gen::Variable var)
	{
		COMPILER_ASSERT("Variable must be optional", var.type.isOpt);
		var.type.isOpt = false;
		if (var.refType == gen::ReferenceType::POINTER)
		{
			IL::Variable offset = simpleConstant(instructions, 1);
			var.ilName = simpleAddToPointer(instructions, var.ilName, offset);
		}
		else
		{
			IL::Type newType = getValueImplementation(var.type);
			IL::Variable isValidFlag = env.createAnonymousVariable(IL::Type::i1);
			var.ilName = simpleCast(instructions, newType, var.ilName);
		}
		return var;
	}

	gen::Variable gen::Generator::assignReference(IL::Program& instructions, gen::Variable lhs, gen::Variable rhs)
	{
		COMPILER_ASSERT("Lhs must be a reference", lhs.type.isRef);
		COMPILER_ASSERT("Rhs cannot be a reference", !rhs.type.isRef);
		if (lhs.refType == gen::ReferenceType::POINTER) 
		{
			lhs.refType = gen::ReferenceType::VALUE;
		}
		lhs.refType = gen::ReferenceType::POINTER;
		lhs.type.isRef = false;
		return assignVariable(instructions, lhs, rhs);
	}
}