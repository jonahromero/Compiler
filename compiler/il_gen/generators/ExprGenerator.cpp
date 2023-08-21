#include "ExprGenerator.h"
#include "VectorUtil.h"
#include "StringUtil.h"
#include "SemanticError.h"
#include "VariantUtil.h"
#include "TargetInfo.h"
#include "FunctionHelpers.h"

ExprGenerator::ExprGenerator(Enviroment& env, TypePtr typeContext)
	: env(env), typeContext(typeContext), gen::Generator(env)
{
}

ExprGenerator ExprGenerator::defaultContext(Enviroment& env)
{
	return ExprGenerator(env, env.types.getPrimitiveType(PrimitiveType::SubType::i16));
}

ExprGenerator ExprGenerator::typedContext(Enviroment& env, TypePtr type)
{
	return ExprGenerator(env, type);
}

ILExprResult ExprGenerator::generateWithCast(Expr::UniquePtr const& expr, TypeInstance outputType)
{
	ILExprResult result = visitChild(expr);
	assertIsAssignableType(expr->sourcePos, result.output.type, outputType);
	result.output = castVariable(result.instructions, result.output, outputType);
	return result;
}

ILExprResult ExprGenerator::generate(Expr::UniquePtr const& expr)
{
	return visitChild(expr);
}

PrimitiveType const*
ExprGenerator::determineBinaryOperandCasts(SourcePosition const& pos,
	PrimitiveType const* lhs,
	Token::Type oper,
	PrimitiveType const* rhs)
{
	using enum PrimitiveType::SubType;
	if (isLogicalOperator(oper))
	{
		return env.types.getPrimitiveType(bool_);
	}
	else if (lhs->subtype == bool_) { return rhs; }
	else if (rhs->subtype == bool_) { return lhs; }
	else if (lhs->size == rhs->size)
	{
		if (lhs->isUnsigned() == rhs->isUnsigned())
		{
			return lhs; // or rhs. theyre the same
		}
		else if (!lhs->isLargestType())
		{
			return env.types.getPrimitiveType(lhs->getPromotedSubType());
		}
		else
		{
			std::string msg = fmt::format(
				"Explicit cast required for the {} of the binary expression. "
				"Beware of a potential overflow combining unsigned/signed types.",
				lhs->isUnsigned() ? "left hand side" : "right hand side"
			);
			throw SemanticError(pos, std::move(msg));
		}
	}
	else
	{
		return lhs->size >= rhs->size ? lhs : rhs;
	}
}

TypeInstance ExprGenerator::determineBinaryReturnType(Token::Type oper, TypeInstance defaultType) const
{
	if (isLogicalOperator(oper) || isRelationalOperator(oper))
		return TypeInstance(env.types.getPrimitiveType(PrimitiveType::SubType::bool_));
	else
		return defaultType;
}

void ExprGenerator::visit(Expr::Binary const& expr)
{
	IL::ILBody instructions;
	auto lhs = visitChild(expr.lhs), rhs = visitChild(expr.rhs);
	util::vector_append(instructions, std::move(lhs.instructions));
	util::vector_append(instructions, std::move(rhs.instructions));

	PrimitiveType const* lhsType  = expectPrimitive(expr.lhs->sourcePos, lhs.output.type);
	PrimitiveType const* rhsType  = expectPrimitive(expr.rhs->sourcePos, rhs.output.type);
	
	TypeInstance castType = determineBinaryOperandCasts(expr.sourcePos, lhsType, expr.oper, rhsType);
	TypeInstance returnType = determineBinaryReturnType(expr.oper, castType);

	if (canDereferenceValue(lhs.output))
		lhs.output = derefVariable(instructions, lhs.output);
	if (canDereferenceValue(rhs.output))
		rhs.output = derefVariable(instructions, rhs.output);

	lhs.output = castVariable(instructions, lhs.output, castType);
	rhs.output = castVariable(instructions, rhs.output, castType);

	gen::Variable out = allocateVariable(instructions, returnType);
	IL::Type ilReturnType = env.getILVariableType(out.ilName);

	returnValue(ILExprResultBuilder{}.withOutput(out)
									 .andInstructions(std::move(instructions))
									 .andInstruction<IL::Binary>(
										 out.ilName, ilReturnType,
										 lhs.output.ilName, expr.oper, rhs.output.ilName
									 ).buildAsTemporary());
}

void ExprGenerator::visit(Expr::Unary const& expr)
{
	IL::ILBody instructions;
	auto rhs = visitChild(expr.expr);
	util::vector_append(instructions, std::move(rhs.instructions));

	PrimitiveType const* rhsType = expectPrimitive(expr.sourcePos, rhs.output.type);
	TypeInstance retType = rhsType;

	if (isLogicalOperator(expr.oper))
	{
		TypeInstance boolType = env.types.getPrimitiveType(PrimitiveType::SubType::bool_);
		castVariable(instructions, rhs.output, boolType);
		retType = boolType;
	}
	gen::Variable out = allocateVariable(instructions, retType);
	IL::Type ilReturnType = env.getILVariableType(out.ilName);

	returnValue(ILExprResultBuilder{}
		.withOutput(out)
		.andInstructions(std::move(instructions))
		.andInstruction<IL::Unary>(
			out.ilName, ilReturnType,
			expr.oper, rhs.output.ilName)
		.buildAsTemporary());
}

void ExprGenerator::visit(Expr::KeyworkFunctionCall const& expr)
{
	auto argument = visitChild(expectOneArgument(expr.sourcePos, &expr.args));

	switch (expr.function) 
	{
	case Token::Type::SIZEOF: 
	{
		IL::ILBody instructions;
		TypeInstance exprType = env.types.getPrimitiveType(PrimitiveType::SubType::u16);
		size_t typeSize = TargetInfo::calculateTypeSizeBytes(argument.output.type);
		gen::Variable out = allocateVariable(instructions, exprType);
		IL::Type ilReturnType = env.getILVariableType(argument.output.ilName);

		returnValue(ILExprResultBuilder{}
			.withOutput(out)
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

void ExprGenerator::visit(Expr::Parenthesis const& expr)
{
	returnValue(visitChild(expr.expr));
}

void ExprGenerator::visit(Expr::Identifier const& expr)
{
	if (env.isValidVariable(expr.ident))
	{
		returnValue(ILExprResultBuilder{}
			.withOutput(env.getVariable(expr.ident))
			.andName(expr.ident)
			.buildAsPersistent());
	}
	else {
		throw SemanticError(expr.sourcePos, util::strBuilder("Use of unknown variable: ", expr.ident));
	}
}

void ExprGenerator::visit(Expr::Literal const& expr)
{
	std::visit(util::OverloadVariant
	{
	[&](u16 const& value) 
	{
		IL::ILBody instructions;
		gen::Variable out = allocateVariable(instructions, typeContext);
		IL::Type ilReturnType = env.getILVariableType(out.ilName);

		returnValue(ILExprResultBuilder{}
			.withOutput(out)
			.andInstructions(std::move(instructions))
			.andInstruction<IL::Assignment>(out.ilName, ilReturnType, std::move(value))
			.buildAsTemporary());
	},
	[&](std::string const& str) { COMPILER_NOT_SUPPORTED; },
	[&](std::monostate) { COMPILER_NOT_REACHABLE; }
	}, expr.literal);
}

// Need to finish
void ExprGenerator::visit(Expr::FunctionCall const& expr)
{
	auto funcPtr = visitChild(expr.lhs);
	FunctionType const* funcType = expectCallable(expr.sourcePos, funcPtr.output.type);
	/* error handling, outputs: funcType, argResults
	std::vector<TypeInstance> argumentTypes;
	auto argResults = util::transform_vector(expr.arguments, 
	[&](Expr::UniquePtr const& arg) {
		auto argResult = visitChild(arg);
		argumentTypes.push_back(argResult.output.type);
		return argResult;
	});
	assertCorrectFunctionCall(expr.sourcePos, funcType.params, argumentTypes);*/
	
	// compile it
	IL::Program instructions;
	gen::Variable result = FunctionCaller(env, funcType).callFunction(instructions, expr);

	returnValue(ILExprResultBuilder{}
		.withOutput(result)
		.andInstructions(std::move(instructions))
		.buildAsTemporary());
}

void ExprGenerator::visit(Expr::Indexing const& expr)
{
	
}

void ExprGenerator::visit(Expr::Cast const& expr)
{

}

void ExprGenerator::visit(Expr::Questionable const& expr)
{
	auto evaluated = visitChild(expr.expr);
}

void ExprGenerator::visit(Expr::MemberAccess const& expr)
{
	ILExprResult lhs = visitChild(expr.lhs);
	BinType::Field const& member = expectMember(expr.sourcePos, lhs.output.type, expr.member);
	if (lhs.getReferenceType() != gen::ReferenceType::POINTER) 
	{
		throw SemanticError(expr.sourcePos, "Cannot perform member access on left hand side.");
	}
	gen::Variable memberBinding = createBindingWithOffset(lhs.instructions, lhs.output, member.type, member.offset);
	returnValue(ILExprResultBuilder{}
		.withOutput(memberBinding)
		.andInstructions(std::move(lhs.instructions))
		.buildAsTemporary(lhs.isTemporary()));
}

void ExprGenerator::visit(Expr::ListLiteral const& expr)
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
void ExprGenerator::visit(Expr::StructLiteral const& expr)
{
	COMPILER_NOT_SUPPORTED;
}

// This is somewhat non sensical. Nothing to compile this to.
void ExprGenerator::visit(Expr::TemplateCall const& expr)
{
	throw SemanticError(expr.sourcePos, "Found unexpected type expression.");
}

void ExprGenerator::visit(Expr::Reference const& expr)
{
	throw SemanticError(expr.sourcePos, "Found unexpected type expression.");
}

void ExprGenerator::visit(Expr::FunctionType const& expr)
{
	throw SemanticError(expr.sourcePos, "Found unexpected type expression.");
}

// These are only allowed within instructions.
void ExprGenerator::visit(Expr::Register const& expr) { COMPILER_NOT_REACHABLE;  }
void ExprGenerator::visit(Expr::Flag const& expr) { COMPILER_NOT_REACHABLE; }
void ExprGenerator::visit(Expr::CurrentPC const& expr) { COMPILER_NOT_REACHABLE; }

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