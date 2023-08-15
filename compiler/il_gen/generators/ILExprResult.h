#pragma once
#include "IL.h"
#include "Enviroment.h"
#include "VariableCreator.h"
#include "VectorUtil.h"

struct ILExprResult
{
	IL::Program instructions;
	const IL::Variable output;
	const TypeInstance type;

	enum class ReferenceType { VALUE, POINTER };

	bool isPointerReferenceType() const { 
		return m_referenceType.has_value() && m_referenceType.value() == ReferenceType::POINTER; 
	}
	bool isValueReferenceType() const {
		return m_referenceType.has_value() && m_referenceType.value() == ReferenceType::POINTER;
	}
	bool isReferenceType() const {
		return isValueReferenceType() || isPointerReferenceType();
	}

	bool isNamed() const { return name.has_value(); }
	std::string_view getName() const { return name.value(); }

private:
	friend class ILExprResultBuilder;
	ILExprResult(IL::Variable output, TypeInstance type)
		: output(std::move(output)), outputType(std::move(type)) {}

	std::optional<ReferenceType> m_referenceType;
	std::optional<std::string_view> m_name;
};

class ILExprResultInstructionBuilder 
{
public:
	ILExprResultInstructionBuilder& andInstruction(IL::UniquePtr instruction) {
		result.instructions.push_back(std::move(instruction));
		return *this;
	}
	ILExprResultInstructionBuilder& andInstructions(IL::Program instructions) {
		util::vector_append(result.instructions, std::move(instructions));
		return *this;
	}
	ILExprResult build() { return std::move(result); }

protected:
	ILExprResult result;
};

class ILExprResultReferenceBuilder : protected ILExprResultInstructionBuilder
{
	ILExprResultReferenceBuilder withExprType(TypeInstance const& instance) {
		return ILExprResult(variable, instance);
	}
private:
	friend class ILExprResultExprTypeBuilder;
	ILExprResultReferenceBuilder(ILExprResult result)
		: result(result) {}
}

class ILExprResultExprTypeBuilder
{
public:
	ILExprResultExprTypeBuilder setOutVariable(IL::Variable variable)
	{
		return ILExprResultExprTypeBuilder{ variable };
	}
private:
	friend class ILExprResultBuilder;
	ILExprResultExprTypeBuilder(IL::Variable variable)
		: variable(variable) {}

	IL::Variable variable;
};

struct ILExprResultBuilder
{
	using RefType = ILExprResultBuilder::ReferenceType;
public:
	ILExprResultInstructionBuilder& createTemporary() 
	{
		return *this;
	}

	ILExprResultInstructionBuilder& createNamedReference(std::string_view name, RefType refType)
	{
		result.m_referenceType = refType;
		result.m_name = name;
		return *this;
	}

	ILExprResultInstructionBuilder& createUnnamedReference(RefType refType) {
		result.m_referenceType = refType;
		return *this;
	}
};
