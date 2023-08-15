#pragma once
#include "IL.h"
#include "Enviroment.h"
#include "VariableCreator.h"
#include "VectorUtil.h"
#include "Generator.h"

struct ILExprResult
{
	IL::Program instructions;
	gen::Variable output;

	gen::ReferenceType getReferenceType() const 
	{
		return output.refType;
	}

	bool isTemporary() const { return m_temporary; }
	bool isNamed() const { return m_name.has_value(); }
	std::string_view getName() const { return m_name.value(); }

private:
	friend class ILExprResultBuilderInstruction;
	friend class ILExprResultBuilderExprType;

	ILExprResult(gen::Variable output)
		: output(std::move(output)) {}

	std::optional<std::string_view> m_name;
	bool m_temporary;
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
	ILExprResult buildAsPersistent() { result.m_temporary = false; return std::move(result); }
	ILExprResult buildAsTemporary() { result.m_temporary = true; return std::move(result); }

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
		ILExprResult result{ gen::Variable{
			variable.value(), refType, type
		}};
		result.m_name = std::move(m_name);
		return ILExprResultBuilderInstruction{ std::move(result) };
	}
protected:
	gen::ReferenceType refType;
	std::optional<IL::Variable> variable; // delayed initialization
	std::optional<std::string_view> m_name;
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
	ILExprResultBuilderOutput& createNamedReference(std::string_view name, gen::ReferenceType refType)
	{
		this->refType = refType;
		m_name = name;
		return *this;
	}

	ILExprResultBuilderOutput& createUnnamedReference(gen::ReferenceType refType) {
		this->refType = refType;
		return *this;
	}
};
