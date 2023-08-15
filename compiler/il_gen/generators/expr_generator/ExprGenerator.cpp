#include "ExprGenerator.h"
#include "ExprGenerator.h"
#include "ExprGenerator.h"
#include "ExprGenerator.h"
#include "ExprGenerator.h"
#include "ExprGenerator.h"
#include "ExprGenerator.h"
#include "ExprGenerator.h"
#include "ExprGenerator.h"
#include "ExprGenerator.h"
#include "ExprGenerator.h"
#include "ExprGenerator.h"
#include "ExprGenerator.h"
#include "ExprGenerator.h"
#include "ExprGenerator.h"
#include "VectorUtil.h"
#include "StringUtil.h"
#include "SemanticError.h"
#include "VariantUtil.h"

ExprGenerator::ExprGenerator(Enviroment& env, TypePtr typeContext)
	: env(env), typeContext(typeContext), gen::Generator(env)
{
}

ExprGenerator ExprGenerator::defaultContext(Enviroment& env)
{
	return ExprGenerator(env, &env.types.getPrimitiveType(PrimitiveType::SubType::i16));
}

ExprGenerator ExprGenerator::typedContext(Enviroment& env, TypePtr type)
{
	return ExprGenerator(env, type);
}

ILExprResult ExprGenerator::generateWithCast(Expr::UniquePtr& expr, TypeInstance outputType)
{
	ILExprResult result = visitChild(expr);
	assertIsAssignableType(expr->sourcePos, result.type, outputType);
	castVariable(result.instructions, result.output, result.type, outputType);
	result.type = outputType;
	return result;
}

ILExprResult ExprGenerator::generate(Expr::UniquePtr& expr)
{
	return visitChild(expr);
}

PrimitiveType const& ExprGenerator::expectPrimitive(SourcePosition const& pos, TypeInstance const& type)
{
	if (type.isOpt)
	{
		throw SemanticError(pos, "Expected a primitive type, but found a maybe type instead.");
	}
	auto primitiveType = type.type->getExactType<PrimitiveType>();
	if ( !primitiveType)
	{
		throw SemanticError(pos, fmt::format(
							"Expected a primitive type, "
							"but found the following instead: {}",
							type.type->name));
	}
	return *primitiveType;
}

FunctionType const& ExprGenerator::expectCallable(SourcePosition const& pos, TypeInstance const& type)
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
	return *functionType;
}

BinType::Field const& ExprGenerator::expectMember(SourcePosition const& pos, TypeInstance const& type, std::string_view member)
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


void ExprGenerator::assertValidFunctionArgType(SourcePosition pos, TypeInstance param, TypeInstance arg) const
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

void ExprGenerator::assertIsAssignableType(SourcePosition pos, TypeInstance dest, TypeInstance src) const
{
	if (dest.type != src.type) 
	{
		throw SemanticError(pos, fmt::format("Cannot initialize an expression of type: {}, when "
											 "an argument of type: {}, is expected.",
											 dest.type->name, src.type->name));
	}
}

void ExprGenerator::assertCorrectFunctionCall(SourcePosition pos, std::vector<TypeInstance> const& params, std::vector<TypeInstance> const& args)
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

void ExprGenerator::assertValidListLiteral(SourcePosition pos, std::vector<TypeInstance> const& elementTypes, TypeInstance expected) const
{
	if (elementTypes.empty()) return;
	for (auto& type : elementTypes) 
	{
		assertIsAssignableType(pos, expected, type);
	}
}

PrimitiveType const&
ExprGenerator::determineBinaryOperandCasts(SourcePosition const& pos,
	PrimitiveType const& lhs,
	Token::Type oper,
	PrimitiveType const& rhs)
{
	using enum PrimitiveType::SubType;
	if (isLogicalOperator(oper))
	{
		return *env.types.getPrimitiveType(bool_);
	}
	else if (lhs.subtype == bool_) { return rhs; }
	else if (rhs.subtype == bool_) { return lhs; }
	else if (lhs.size == rhs.size)
	{
		if (lhs.isUnsigned() == rhs.isUnsigned())
		{
			return lhs; // or rhs. theyre the same
		}
		else if (!lhs.isLargestType())
		{
			return *env.types.getPrimitiveType(lhs.getPromotedSubType());
		}
		else
		{
			std::string msg = fmt::format(
				"Explicit cast required for the {} of the binary expression. "
				"Beware of a potential overflow combining unsigned/signed types.",
				lhs.isUnsigned() ? "left hand side" : "right hand side"
			);
			throw SemanticError(pos, std::move(msg));
		}
	}
	else
	{
		return lhs.size >= rhs.size ? lhs : rhs;
	}
}

void ExprGenerator::visit(Expr::Binary& expr)
{
	IL::ILBody instructions;
	auto lhs = visitChild(expr.lhs), rhs = visitChild(expr.rhs);
	util::vector_append(instructions, std::move(lhs.instructions));
	util::vector_append(instructions, std::move(rhs.instructions));

	auto& lhsType	 = expectPrimitive(expr.lhs->sourcePos, lhs.type);
	auto& rhsType	 = expectPrimitive(expr.rhs->sourcePos, rhs.type);
	auto& castType	 = determineBinaryOperandCasts(lhsType, expr.oper, rhsType);
	auto& returnType = TypeInstance(&(isLogicalOperator(expr.oper) || isRelationalOperator(expr.oper) ?
						env.types.getPrimitiveType(PrimitiveType::SubType::bool_) : castType));

	if (canDereferenceValue(lhs.output, lhs.type))
		lhs.output = derefVariable(instructions, lhs.output, lhs.type);
	if (canDereferenceValue(rhs.output, rhs.type))
		rhs.output = derefVariable(instructions, rhs.output, rhs.type);

	lhs.output = castVariable(instructions, lhs.output, lhs.type, TypeInstance(&castType));
	rhs.output = castVariable(instructions, rhs.output, rhs.type, TypeInstance(&castType));

	gen::Variable out = allocateVariable(instructions, TypeInstance(&returnType));
	IL::Type ilReturnType = env.getILAliasType(out.ilName);

	returnValue(ILExprResultBuilder{}.createUnnamedReference(out.refType)
									 .withOutputAt(out.ilName).withExprType(TypeInstance(&returnType))
									 .andInstructions(std::move(instructions))
									 .andInstruction<IL::Binary>(
										 out.ilName, ilReturnType,
										 lhs.output, expr.oper, rhs.output
									 ).buildAsTemporary());
}

void ExprGenerator::visit(Expr::Unary& expr)
{
	IL::ILBody instructions;
	auto rhs = visitChild(expr.expr);
	util::vector_append(instructions, std::move(rhs.instructions));
	auto& rhsType = expectPrimitive(expr.rhs->sourcePos, rhs.type);
	auto& retType = rhsType;

	if (isLogicalOperator(expr.oper)) 
	{
		retType = env.types.getPrimitiveType(PrimitiveType::SubType::bool_);
		rhs.output = castVariable(instructions, IL::Type::i1, rhs.output);
	}
	gen::Variable out = allocateVariable(instructions, TypeInstance(&retType));
	IL::Type ilReturnType = env.getILAliasType(out.ilName);

	returnValue(ILExprResultBuilder{}
		.createUnnamedReference(out.refType)
		.withOutputAt(out.ilName).withExprType(TypeInstance(&retType))
		.andInstructions(std::move(instructions))
		.andInstruction<IL::Unary>(
			out.ilName, ilReturnType,
			expr.oper, rhs.output)
		.buildAsTemporary());
}

void ExprGenerator::visit(Expr::KeyworkFunctionCall& expr)
{
	if (expr.args.size() != 1)
	{
		std::string msg = fmt::format("{} expected a single argument.", 
									  tokenTypeToStr(expr.function));
		throw SemanticError(expr.sourcePos, std::move(msg));
	}
	auto argument = visitChild(expr.args[0]);
	switch (expr.function) 
	{
	case Token::Type::SIZEOF: 
	{
		IL::Instruction instructions;
		TypeInstance exprType = env.types.getPrimitiveType(PrimitiveType::SubType::u16);
		size_t typeSize = calculateTypeSizeBytes(argument.type);
		gen::Variable out = allocateVariable(instructions, exprType);
		IL::Type ilReturnType = env.getILAliasType(result.ilName);

		returnValue(ILExprResultBuilder{}
			.createUnnamedReference(out.refType)
			.withOutputAt(out.ilName)
			.withExprType(exprType)
			.andInstructions(std::move(instructions))
			.andInstruction<IL::Assignment>(out.ilName, ilReturnType, int(typeSize))
			.buildAsTemporary());
		break;
	}
	case Token::Type::DEREF:
		COMPILER_NOT_REACHABLE;
		break;
	}
}

void ExprGenerator::visit(Expr::Parenthesis& expr)
{
	returnValue(visitChild(expr.expr));
}

void ExprGenerator::visit(Expr::Identifier& expr)
{
	if (env.isValidVariable(expr.ident)) 
	{
		returnValue(ILExprResultBuilder{}
			.createNamedReference(expr.ident, env.getVariableReferenceType(expr.ident))
			.withOutputAt(env.getVariableILAlias(expr.ident))
			.withExprType(env.getVariableType(expr.ident))
			.buildAsPersistent());
	}
	else {
		throw SemanticError(expr.sourcePos, util::strBuilder("Use of unknown variable: ", expr.ident));
	}
}

void ExprGenerator::visit(Expr::Literal& expr)
{
	std::visit(util::OverloadVariant
	{
	[&](u16 const& value) 
	{
		IL::ILBody instructions;
		gen::Variable out = allocateVariable(instructions, typeContext);
		IL::Type ilReturnType = env.getILAliasType(out.ilName);

		returnValue(ILExprResultBuilder{}
			.createUnnamedReference(out.refType)
			.withOutputAt(out.ilName)
			.withExprType(typeContext)
			.andInstructions(std::move(instructions))
			.andInstruction<IL::Assignment>(out.ilName, ilReturnType, std::move(value))
			.buildAsTemporary());
	},
	[&](std::string const& str) { COMPILER_NOT_SUPPORTED; },
	[&](std::monostate) { COMPILER_NOT_REACHABLE; }
	}, expr.literal);
}

// Need to finish
void ExprGenerator::visit(Expr::FunctionCall& expr) 
{
	// error handling, outputs: funcType, argResults
	auto funcPtr = visitChild(expr.lhs);
	FunctionType const& funcType = expectCallable(funcPtr.type);
	std::vector<TypeInstance> argumentTypes;
	auto argResults = util::transform_vector(expr.arguments, 
	[&](Expr::UniquePtr const& arg) {
		auto argResult = visitChild(arg);
		argumentTypes.push_back(argResult.type);
		return argResult;
	});
	assertCorrectFunctionCall(expr.sourcePos, funcType.params, argumentTypes);
	
	// compile it
	IL::ILBody instructions;
	std::vector<IL::Value> arguments;
	for (auto& arg : argResults) 
	{
		arguments.push_back(arg.output);
	}
	if (shouldPassInReturnValue(funcType))
	{
		size_t bufferSize = calculateTypeSizeBytes(funcType.returnType);
		gen::Variable returnBuffer = simpleAllocate(instructions, bufferSize);
		arguments.push_back(returnBuffer.ilName);
	}
	if (funcPtr.isNamed()) 
	{
		instructions.push_back(IL::makeIL<IL::FunctionCall>(funcPtr.getName(), std::move(arguments)));
	}
	else 
	{
		instructions.push_back(IL::makeIL<IL::FunctionCall>(funcPtr.output, std::move(arguments)));
	}
	returnValue(ILExprResultBuilder{}.createUnnamedReference()
									 .withOutputAt()
									 .withExprType(funcType.returnType)
									 .andInstructions(std::move(instructions))
									 .buildAsTemporary());
}

void ExprGenerator::visit(Expr::Indexing& expr) 
{
	
}

void ExprGenerator::visit(Expr::Cast& expr)
{

}

void ExprGenerator::visit(Expr::Questionable& expr)
{
	auto evaluated = visitChild(expr.expr);
}

void ExprGenerator::visit(Expr::MemberAccess& expr)
{
	ILExprResult lhs = visitChild(expr.lhs);
	BinType::Field const& member = expectMember(expr.sourcePos, lhs.type, expr.member);
	if (lhs.getReferenceType() != gen::ReferenceType::POINTER) 
	{
		throw SemanticError(expr.sourcePos, "Cannot perform member access on left hand side.");
	}
	switch (lhs.getReferenceType())
	{
	case gen::ReferenceType::POINTER:
		lhs.output = addToPointer(lhs.instructions, lhs.output, int(member.offset));
		break;
	case gen::ReferenceType::VALUE:
		if (lhs.isTemporary()) 
		{
		
		}
		else 
		{
		
		}
	}
	auto builder = ILExprResultBuilder{}.createUnnamedReference(lhs.getReferenceType())
									    .withOutputAt(lhs.output)
										.withExprType(member.type)
										.andInstructions(std::move(lhs.instructions));
	returnValue(lhs.isTemporary() ? builder.buildAsTemporary() : builder.buildAsPersistent());
}

void ExprGenerator::visit(Expr::ListLiteral& expr) 
{
	COMPILER_NOT_SUPPORTED;
	auto elements = util::transform_vector(expr.elements, [&](auto& expr) 
	{
		return visitChild(expr);
	});
	// default initialize the array
	if (elements.empty()) 
	{
		typeContext->size;
	}
	else 
	{
		
	}
}
void ExprGenerator::visit(Expr::StructLiteral& expr) 
{
	COMPILER_NOT_SUPPORTED;
}

// This is somewhat non sensical. Nothing to compile this to.
void ExprGenerator::visit(Expr::TemplateCall& expr) 
{
	throw SemanticError(expr.sourcePos, "Found unexpected type expression.");
}

void ExprGenerator::visit(Expr::Reference& expr) 
{
	throw SemanticError(expr.sourcePos, "Found unexpected type expression.");
}

void ExprGenerator::visit(Expr::FunctionType& expr)
{
	throw SemanticError(expr.sourcePos, "Found unexpected type expression.");
}

// These are only allowed within instructions.
void ExprGenerator::visit(Expr::Register& expr) { COMPILER_NOT_REACHABLE;  }
void ExprGenerator::visit(Expr::Flag& expr) { COMPILER_NOT_REACHABLE; }
void ExprGenerator::visit(Expr::CurrentPC& expr) { COMPILER_NOT_REACHABLE; }

bool ExprGenerator::isLogicalOperator(Token::Type oper)
{
	using enum Token::Type;
	static constexpr std::array<Token::Type, 9> logicals = {
		AND, OR, BANG
	};
	return std::find(logicals.begin(), logicals.end(), oper) != logicals.end();
}

bool ExprGenerator::isRelationalOperator(Token::Type oper)
{
	using enum Token::Type;
	static constexpr std::array<Token::Type, 9> relationals = {
		EQUAL_EQUAL, NOT_EQUAL, LESS, LESS_EQUAL,
		GREATER, GREATER_EQUAL
	};
	return std::find(relationals.begin(), relationals.end(), oper) != relationals.end();
}

bool ExprGenerator::isArithmeticOperator(Token::Type oper)
{
	using enum Token::Type;
	static constexpr std::array<Token::Type, 11> arithmetics = {
		PLUS, MINUS, SLASH, STAR, MODULO,
		SHIFT_LEFT, SHIFT_RIGHT,
		BIT_OR, BIT_AND, BIT_XOR, BIT_NOT
	};
	return std::find(arithmetics.begin(), arithmetics.end(), oper) != arithmetics.end();
}