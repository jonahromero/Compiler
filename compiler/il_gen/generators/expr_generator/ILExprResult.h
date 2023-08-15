#pragma once
#include "IL.h"
#include "Enviroment.h"
#include "VariableCreator.h"
#include "VectorUtil.h"

namespace gen 
{
	enum class ReferenceType 
	{
		VALUE, POINTER, DOUBLE_POINTER
	};

	class Variable
	{
	public:
		IL::Variable ilName;
		ReferenceType refType;
	};
}

struct ILExprResult
{
	IL::Program instructions;
	const IL::Variable output;
	const TypeInstance type;

	IL::ReferenceType getReferenceType() const 
	{
		return m_referenceType.value();
	}

	bool isReferenceType() const 
	{
		return m_referenceType.has_value();
	}

	bool isNamed() const { return name.has_value(); }
	std::string_view getName() const { return name.value(); }

private:
	friend class ILExprResultBuilderInstruction;
	friend class ILExprResultBuilderExprType;

	ILExprResult(IL::Variable output, TypeInstance type)
		: output(std::move(output)), outputType(std::move(type)) {}

	std::optional<IL::ReferenceType> m_referenceType;
	std::optional<std::string_view> m_name;
};

// The following expresses a builder pattern using a series of classes
// in order to ensure that ILExprResult is always constructed correctly
// and that the sequence of build methods used is always a specific order.
// This forces a linguistic sentence to be built, and makes the construction
// of an ILExprResult at the callsite much more understandable. 

class ILExprResultBuilderInstruction
{
public:
	template<typename T, typename...Args>
	ILExprResultBuilderInstruction& andInstruction(Args...instructionArgs) {
		result.instructions.push_back(IL::makeIL<T>(std::forward<Args>(instructionArgs)...));
		return *this;
	}
	ILExprResultBuilderInstruction& andInstruction(IL::UniquePtr instruction) {
		result.instructions.push_back(std::move(instruction));
		return *this;
	}
	ILExprResultBuilderInstruction& andInstructions(IL::Program instructions) {
		util::vector_append(result.instructions, std::move(instructions));
		return *this;
	}
	ILExprResult build() { return std::move(result); }

private:
	friend class ILExprResultBuilderExprType;
	ILExprResultBuilderInstruction(ILExprResult result)
		: result(std::move(result)) {}
	ILExprResult result;
};

class ILExprResultBuilderExprType
{
public:
	ILExprResultBuilderInstruction withExprType(TypeInstance type)
	{
		ILExprResult result{variable, type};
		result.m_name = std::move(m_name);
		result.m_referenceType = std::move(m_referenceType);
		return ILExprResultBuilderInstruction{ std::move(result) };
	}
protected:
	std::optional<IL::ReferenceType> m_referenceType;
	std::optional<std::string_view> m_name;
	IL::Variable variable;
};

class ILExprResultBuilderOutput : protected ILExprResultBuilderExprType
{
public:
	ILExprResultBuilderExprType& withOutputAt(IL::Variable variable)
	{
		this->variable = variable;
		return *this;
	}
};

struct ILExprResultBuilder : protected ILExprResultBuilderOutput
{
public:
	ILExprResultBuilderOutput& createValue()
	{
		return *this;
	}

	ILExprResultBuilderOutput& createNamedReference(std::string_view name, IL::ReferenceType refType)
	{
		result.m_referenceType = refType;
		result.m_name = name;
		return *this;
	}

	ILExprResultBuilderOutput& createUnnamedReference(IL::ReferenceType refType) {
		result.m_referenceType = refType;
		return *this;
	}
};
