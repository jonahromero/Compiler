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
	: env(env), typeContext(typeContext)
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

ILExprResult ExprGenerator::generateWithCast(Expr::UniquePtr& expr, IL::Type outputType)
{
	ILExprResult result = visitChild(expr);
	
	if (result.producesObjectRef()) 
	{
		result.output = derefVariable(result.instructions, expr->sourcePos, result.output, result.getResultingRefType().type);
	}
	result.output = castVariable(result.instructions, expr->sourcePos, outputType, result.output);
	return result;
}

ILExprResult ExprGenerator::generate(Expr::UniquePtr& expr)
{
	return visitChild(expr);
}

IL::Variable ExprGenerator::simpleAllocate(IL::Program& instructions, size_t size)
{
	IL::Variable result = createNewILVariable(IL::Type::u8_ptr);
	instructions.push_back(IL::makeIL<IL::Allocate>(result, int(size)));
	return result;
}

IL::Variable ExprGenerator::simpleDeref(IL::Program& instrs, IL::Type type, IL::Variable ptr)
{
	COMPILER_ASSERT("Cannot dereference a non-pointer type", env.getILAliasType(var) == IL::Type::u8_ptr);
	IL::Variable result;
	instrs.push_back(IL::makeIL<IL::Deref>(result, rtpe, ptr));
	return result;
}

IL::Variable ExprGenerator::derefVariable(
	IL::Program& instructions,
	IL::Variable ptr,
	IL::ReferenceType refType,
	PrimitiveType const* derefType,
	bool isOpt)
{
	COMPILER_ASSERT("Cannot dereference a value reference type", refType != IL::ReferenceType::VALUE);
	if (refType == IL::ReferenceType::DOUBLE_POINTER) {
		ptr = simpleDeref(instructions, IL::Type::u8_ptr, ptr);
	}
	if (isOpt)
	{
		ptr = addToPointer(instructions, ptr, 1);
	}
	IL::Type instantiatedType = env.types.compileType(TypeInstance(derefType));
	return simpleDeref(instantiatedType, ptr);
}

IL::Variable ExprGenerator::addToPointer(IL::Program& instructions, IL::Variable ptr, int offset)
{
	IL::Variable lhs = ptr, rhs = env.createAnonymousVariable(IL::Type::u16);
	IL::Variable result = env.createAnonymousVariable();
	instructions.push_back(IL::makeIL<IL::Assignment>(rhs, IL::Type::u16, offset));
	instructions.push_back(IL::makeIL<IL::Binary>(var, IL::Type::u8_ptr, lhs, Token::Type::PLUS, rhs));
	return result;
}

IL::Variable ExprGenerator::castVariable(IL::Program& instrs, IL::Type newType, IL::Variable var)
{
	if (env.getILAliasType(var) == newType) return var;
	auto castedVariable = env.createAnonymousVariable(newType);
	instrs.emplace_back(IL::makeIL<IL::Cast>(castedVariable, newType, var));
	return castedVariable;
}

IL::Variable ExprGenerator::createNewILVariable(IL::Type type) const
{
	return env.createAnonymousVariable(type);
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
		const char* msg = "Expected a primitive type, but found the following instead: {}";
		throw SemanticError(fmt::format(msg, type.type->name));
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
		const char* msg = "Expected a callable type, but found the following instead: {}";
		throw SemanticError(fmt::format(msg, type.type->name));
	}
	return *functionType;
}

BinType::Field const& ExprGenerator::expectMember(SourcePosition const& pos, TypeInstance const& type, std::string_view member)
{
	auto* bin = type.type->getExactType<BinType>();
	if (!bin) {
		throw SemanticError(expr.sourcePos, fmt::format("Cannot perform member access on the following type: {}", type.type->name));
	}
	auto it = std::find_if(bin->members.begin(), bin->members.end(), [member](auto& field) {
		return member == field.name;
	});
	if (it == bin->members.end()) {
		throw SemanticError(expr.sourcePos, fmt::format("No member \"{}\" exists in the type: {}", expr.member, bin->name));
	}
	return *it;
}


void ExprGenerator::assertValidFunctionArgType(SourcePosition pos, TypeInstance param, TypeInstance arg) const
{
	if (param.type != arg.type) {
		throw SemanticError(pos, fmt::format("Passed in an argument of: {}, however, "
											 "the function expected an argument of type: {}"), 
											 param.type->name, arg.type->name);
	}
	if (param.isRef && !arg.isRef) {
		throw SemanticError(pos, "Argument passed is not a reference type");
	}
}

void ExprGenerator::assertIsAssignableType(SourcePosition pos, TypeInstance dest, TypeInstance src) const
{
	if (param.type != arg.type) 
	{
		throw SemanticError(pos, fmt::format("Cannot initialize an expression of type: {}, when "
											 "an argument of type: {}, is expected."),
											 dest.type->name, src.type->name);
	}
}

void ExprGenerator::assertCorrectFunctionCall(SourcePosition pos, std::vector<TypeInstance> const& params, std::vector<TypeInstance> const& args)
{
	if (params.size() != arguments.size()) {
		throw SemanticError(pos,
			fmt::format("Function expected {} arguments, however, it recieved {} instead.",
				params.size(), arguments.size()
			));
	}
	for (size_t i = 0; i < params.size(); i++) 
	{
		assertValidFunctionArgType(sourcePos, params[i], args[i]);
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
		return env.types.getPrimitiveType(bool_);
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
			return env.types.getPrimitiveType(lhs.getPromotedSubType());
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
	auto& returnType = isLogicalOperator(expr.oper) || isRelationalOperator(expr.oper) ?
						env.types.getPrimitiveType(PrimitiveType::SubType::bool_) : castType;

	if (lhs.isReferenceType() && lhs.getReferenceType() != IL::ReferenceType::VALUE) 
		derefVariable(instructions, lhs.output, lhs.getReferenceType(), lhsType, lhs.type.isOpt);
	if (rhs.isReferenceType() && rhs.getReferenceType() != IL::ReferenceType::VALUE)
		derefVariable(instructions, rhs.output, rhs.getReferenceType(), rhsType, rhs.type.isOpt);

	lhs.output = castVariable(instructions, env.types.compileType(TypeInstance(&castType)), lhs.output);
	rhs.output = castVariable(instructions, env.types.compileType(TypeInstance(&castType)), rhs.output);

	IL::Type ilReturnType = env.types.compileType(TypeInstance(&returnType));
	IL::Variable binaryOutput = createNewILVariable(ilReturnType);

	returnValue(ILExprResultBuilder{}.createTemporary()
									 .withOutputAt(binaryOutput).withExprType(exprType)
									 .andInstructions(std::move(instructions))
									 .andInstruction<IL::Binary>(
										 binaryOutput, ilReturnType,
										 lhs.output, expr.oper, rhs.output
									 ).build());
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

	IL::Type ilReturnType = env.types.compileType(TypeInstance(&rhsType));
	IL::Variable unaryOutput = env.createAnonymousVariable(ilReturnType);
	returnValue(ILExprResultBuilder{}.createTemporary()
									 .withOutputAt(unaryOutput).withExprType(TypeInstance(&rhsType))
									 .andInstructions(std::move(instructions))
									 .andInstruction<IL::Unary>(
										 unaryOutput, ilReturnType,
									 	 expr.oper, rhs.output
									 ).build());
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
		size_t typeSize = env.types.calculateTypeSize(argument.type);
		IL::Type ilReturnType = IL::Type::u16;
		IL::Variable sizeOutput = createNewILVariable(ilReturnType);
		returnValue(ILExprResultBuilder{}.createTemporary()
			.withOutputAt(sizeOutput)
			.withExprType(env.types.getPrimitiveType(PrimitiveType::SubType::u16))
			.andInstruction<IL::Assignment>(sizeOutput, ilReturnType, int(typeSize)));
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
			.build());
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
		auto ilReturnType = env.types.compileType(typeContext);
		auto output = createNewILVariable(ilReturnType);
		returnValue(ILExprResultBuilder{}
			.createTemporary()
			.withOutputAt(output)
			.withExprType(typeContext)
			.andInstruction<IL::Assignment>(output, ilReturnType, std::move(value))
			.build());
	},
	[&](std::string const& str) { COMPILER_NOT_SUPPORTED; },
	[&](std::monostate) { COMPILER_NOT_REACHABLE; }
	}, expr.literal);
}

// Need to finish
void ExprGenerator::visit(Expr::FunctionCall& expr) 
{
	auto funcPtr = visitChild(expr.lhs);
	FunctionType* funcType = expectCallable(funcPtr.type);
	std::vector<TypeInstance> argumentTypes;
	auto argResults = util::transform_vector(expr.arguments, 
	[&](Expr::UniquePtr const& arg) {
		auto argResult = visitChild(arg);
		argumentTypes.push_back(argResult.type);
		return argResult;
	});
	assertCorrectFunctionCall(expr.sourcePos, funcType->params, argumentTypes);

	IL::ILBody instructions;
	if (funcType->passesReturnType)
	{
		IL::Variable returnStorage = simpleAllocate(instructions, env.types.calculateTypeSize(funcType->returnType));
	}
	if (funcPtr.isNamed()) 
	{
		auto name = funcPtr.getName();

	}
	else {
			
	}
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
	if (lhs.getReferenceType() != IL::ReferenceType::POINTER) 
	{
		throw SemanticError(expr.sourcePos, "Cannot perform member access on left hand side.");
	}
	lhs.output = addToPointer(lhs.instructions, lhs.output, int(member.offset));
	returnValue(ILExprResultBuilder{}.createUnnamedReference(IL::ReferenceType::POINTER)
									 .withOutputAt(result.output)
									 .withExprType(member.type)
									 .andInstructions(std::move(lhs.instructions))
									 .build());
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

bool ExprGenerator::isPrimitiveType(TypePtr type)
{
	return type->getExactType<PrimitiveType>();
}

IL::Type ExprGenerator::getReferenceTypeImplementation(gen::ReferenceType refType, size_t size)
{
	switch (refType) 
	{
	case gen::ReferenceType::VALUE:
		switch ()
		{
		}
		break;
	case gen::ReferenceType::POINTER:
		return IL::Type::u8_ptr;
	case gen::ReferenceType::DOUBLE_POINTER:
		return IL::Type::u8_ptr;
	}
}

gen::Variable ExprGenerator::allocateVariable(IL::Program& instructions, TypeInstance type)
{
	size_t typeSize = type.type->size + (type.isOpt ? 1 : 0);
	
	if (type.isRef && !type.isOpt) 
	{
		IL::Type impl = getReferenceTypeImplementation(gen::ReferenceType::POINTER);
		return gen::Variable
		{
			env.createAnonymousVariable(impl)
			gen::ReferenceType::POINTER,
		}
	}
	else if(typeSize > 2)
	{
		IL::Variable buffer = simpleAllocate(instructions, typeSize);
		return gen::Variable {
			buffer, gen::ReferenceType::POINTER
		};
	}
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