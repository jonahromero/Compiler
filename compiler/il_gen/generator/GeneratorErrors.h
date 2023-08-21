
#pragma once
#include "Types.h"
#include "IL.h"

namespace gen
{
	class GeneratorErrors
	{
	protected:
		void assertValidFunctionArgType(SourcePosition pos, TypeInstance param, TypeInstance arg) const;
		void assertIsAssignableType(SourcePosition pos, TypeInstance dest, TypeInstance src) const;
		void assertValidListLiteral(SourcePosition pos, std::vector<TypeInstance> const& elementTypes, TypeInstance expected) const;
		void assertCorrectFunctionCall(SourcePosition pos, std::vector<TypeInstance> const& params, std::vector<TypeInstance> const& args);

		PrimitiveType const* expectPrimitive(SourcePosition const& pos, TypeInstance const& type);
		FunctionType const* expectCallable(SourcePosition const& pos, TypeInstance const& type);
		BinType::Field const& expectMember(SourcePosition const& pos, TypeInstance const& type, std::string_view name);
		Expr::UniquePtr const& expectOneArgument(SourcePosition const& pos, std::vector<Expr::UniquePtr> const* args) const;
	};
}