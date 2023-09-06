#include "GeneratorErrors.h"

#include "GeneratorErrors.h"
#include "CompilerError.h"
#include "SemanticError.h"

namespace gen 
{
	PrimitiveType const* GeneratorErrors::expectPrimitive(SourcePosition const& pos, TypeInstance const& type)
	{
		if (type.isOpt)
		{
			throw SemanticError(pos, "Expected a primitive type, but found a maybe type instead.");
		}
		auto primitiveType = type.type->getExactType<PrimitiveType>();
		if (!primitiveType)
		{
			throw SemanticError(pos, fmt::format(
				"Expected a primitive type, "
				"but found the following instead: {}",
				type.type->name));
		}
		return primitiveType;
	}

	FunctionType const* GeneratorErrors::expectCallable(SourcePosition const& pos, TypeInstance const& type)
	{
		if (type.isOpt)
		{
			throw SemanticError(pos, "Expected a callable type, but found a maybe type instead.");
		}
		auto functionType = type.type->getExactType<FunctionType>();
		if (!functionType)
		{
			throw SemanticError(pos, fmt::format(
				"Expected a callable type, "
				"but found the following instead: {}",
				type.type->name));
		}
		return functionType;
	}

	BinType::Field const& GeneratorErrors::expectMember(SourcePosition const& pos, TypeInstance const& type, std::string_view member)
	{
		auto* bin = type.type->getExactType<BinType>();
		if (!bin) {
			throw SemanticError(pos, fmt::format("Cannot perform member access on the following type: {}", type.type->name));
		}
		auto it = std::find_if(bin->members.begin(), bin->members.end(), [member](auto& field) {
			return member == field.name;
			});
		if (it == bin->members.end()) {
			throw SemanticError(pos, fmt::format("No member \"{}\" exists in the type: {}", member, bin->name));
		}
		return *it;
	}

	Expr::UniquePtr const& GeneratorErrors::expectOneArgument(SourcePosition const& pos, std::vector<Expr::UniquePtr> const* args) const
	{
		if (args->size() != 1)
		{
			std::string msg = fmt::format("Expected a single argument, but found {}.", args->size());
			throw SemanticError(pos, std::move(msg));
		}
		return (*args)[0];
	}


	void GeneratorErrors::assertValidFunctionArgType(SourcePosition pos, TypeInstance param, TypeInstance arg) const
	{
		if (param.type != arg.type) {
			throw SemanticError(pos, fmt::format("Passed in an argument of: {}, however, "
				"the function expected an argument of type: {}",
				param.type->name, arg.type->name));
		}
		if (param.isRef && !arg.isRef) {
			throw SemanticError(pos, "Argument passed is not a reference type");
		}
	}

	void GeneratorErrors::assertIsAssignableType(SourcePosition pos, TypeInstance dest, TypeInstance src) const
	{
		if (dest.isMut) 
		{
			throw SemanticError(pos, 
				fmt::format("Cannot assign a variable of mutable "
							"type. Occured when assigning: {}, to {}", 
							dest.type->name, src.type->name));
		}
		if (dest.type != src.type)
		{
			throw SemanticError(pos, fmt::format("Cannot initialize an expression of type: {}, when "
				"an argument of type: {}, is expected.",
				dest.type->name, src.type->name));
		}
	}

	void GeneratorErrors::assertIsCastableType(SourcePosition pos, TypeInstance src, TypeInstance cast) const
	{
		auto srcPrimitive = src.type->getExactType<PrimitiveType>();
		auto castPrimitive = src.type->getExactType<PrimitiveType>();
		// You can only cast primitive types to one another
		//if (!srcPrimitive || !castPrimitive)
		{
			//throw SemanticError(pos, fmt::format("Cannot cast an expression of type: {}, to {}.",
				//src.type->name, cast.type->name));
		}
	}

	ListType const* GeneratorErrors::expectListType(SourcePosition const& pos, TypeInstance const& type)
	{
		if (auto arrayType = type.type->getExactType<ListType>()) {
			return arrayType;
		}
		throw SemanticError(pos, 
			fmt::format("Expected a list type, however, it recieved {} instead.", type.type->name));
	}

	void GeneratorErrors::assertCorrectFunctionCall(SourcePosition pos, std::vector<TypeInstance> const& params, std::vector<TypeInstance> const& args)
	{
		if (params.size() != args.size()) {
			throw SemanticError(pos,
				fmt::format("Function expected {} arguments, however, it recieved {} instead.",
					params.size(), args.size()
				));
		}
		for (size_t i = 0; i < params.size(); i++)
		{
			assertValidFunctionArgType(pos, params[i], args[i]);
		}
	}

	void GeneratorErrors::assertValidListLiteral(SourcePosition pos, std::vector<TypeInstance> const& elementTypes, TypeInstance expected) const
	{
		if (elementTypes.empty()) return;
		for (auto& type : elementTypes)
		{
			assertIsAssignableType(pos, expected, type);
		}
	}
}