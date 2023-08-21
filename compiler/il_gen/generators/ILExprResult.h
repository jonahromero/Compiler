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
	friend class ILExprResultBuilder;
	friend class ILExprResultBuilderFinalizer;

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

class ILExprResultBuilderFinalizer
{
public:
	template<typename T, typename...Args>
	ILExprResultBuilderFinalizer& andInstruction(Args...instructionArgs) {
		result.instructions.push_back(IL::makeIL<T>(std::forward<Args>(instructionArgs)...));
		return *this;
	}
	ILExprResultBuilderFinalizer& andName(std::string_view name) {
		result.m_name = name;
		return *this;
	}
	ILExprResultBuilderFinalizer& andInstruction(IL::UniquePtr instruction) {
		result.instructions.push_back(std::move(instruction));
		return *this;
	}
	ILExprResultBuilderFinalizer& andInstructions(IL::Program instructions) {
		util::vector_append(result.instructions, std::move(instructions));
		return *this;
	}
	ILExprResult buildAsPersistent() { 
		result.m_temporary = false; return std::move(result); 
	}
	ILExprResult buildAsTemporary(bool flag = true) { 
		result.m_temporary = flag; return std::move(result); 
	}

private:
	friend class ILExprResultBuilder;
	ILExprResultBuilderFinalizer(ILExprResult result)
		: result(std::move(result)) {}

	ILExprResult result;
};

class ILExprResultBuilder
{
public:
	ILExprResultBuilder() = default;

	ILExprResultBuilderFinalizer withOutput(gen::Variable variable)
	{
		return ILExprResultBuilderFinalizer(ILExprResult{variable});
	}
};
