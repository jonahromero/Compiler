#include "ExprGenerator.h"
#include "VectorUtil.h"
#include "StringUtil.h"
#include "SemanticError.h"
#include "VariantUtil.h"
#include "TargetInfo.h"
#include "FunctionHelpers.h"

ExprGenerator::ExprGenerator(Enviroment& env, TypePtr typeContext)
	: env(env), typeContext(typeContext), gen::GeneratorToolKit(env)
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
	assertIsCastableType(expr->sourcePos, result.output.type, outputType);
	if (result.output.type != outputType)
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

ListType const* ExprGenerator::determineListLiteralType(SourcePosition const& pos, std::vector<ILExprResult> const& elements)
{
	size_t elementCount = elements.size();
	if (elementCount == 0) {
		// deduce based on context
		if (auto listType = typeContext->getExactType<ListType>()) {
			return listType;
		}
		throw SemanticError(pos, "Unable to determine the type of empty list.");
	}
	TypePtr elementType = elements[0].output.type.type;
	for (auto& element : elements) {
		if (element.output.type.type != elementType) 
		{
			throw SemanticError(pos, fmt::format("Not all types in the list literal are the same. "
												 "Found a {} and {}, which are contradictory.",
												 elementType->name, element.output.type.type->name));
		}
	}
	return env.types.addArray(TypeInstance(elementType), elementCount)->getExactType<ListType>();
}

std::vector<ILExprResult> ExprGenerator::visitChild(std::vector<Expr::UniquePtr> const& args)
{
	return util::transform_vector(args, [&](Expr::UniquePtr const& arg) {
		return visitChild(arg);
	});
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

	lhs.output = getDataTypeAsValue(instructions, lhs.output);
	rhs.output = getDataTypeAsValue(instructions, rhs.output);
	if (lhs.output.type != castType) lhs.output = castVariable(instructions, lhs.output, castType);
	if (rhs.output.type != castType) rhs.output = castVariable(instructions, rhs.output, castType);

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
	rhs.output = getDataTypeAsValue(instructions, rhs.output);
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
	Expr::UniquePtr const& arg = expectOneArgument(expr.sourcePos, &expr.args);
	TypeInstance u16TypeInstance = env.types.getPrimitiveType(PrimitiveType::SubType::u16);
	TypeInstance u8TypeInstance = env.types.getPrimitiveType(PrimitiveType::SubType::u8);

	switch (expr.function) 
	{
	case Token::Type::SIZEOF: 
	{
		IL::ILBody instructions;
		size_t typeSize = TargetInfo::calculateTypeSizeBytes(env.types.instantiateType(arg));
		gen::Variable out = allocateVariable(instructions, u16TypeInstance);
		IL::Type ilReturnType = env.getILVariableType(out.ilName);

		returnValue(ILExprResultBuilder{}
			.withOutput(out)
			.andInstructions(std::move(instructions))
			.andInstruction<IL::Assignment>(out.ilName, ilReturnType, int(typeSize))
			.buildAsTemporary());
		break;
	}
	case Token::Type::REF: {
		auto result = ExprGenerator::typedContext(env, u16TypeInstance.type).generateWithCast(arg, u16TypeInstance);
		result.output = getDataTypeAsValue(result.instructions, result.output);
		IL::Variable pointer = simpleCast(result.instructions, getPointerImplementation(), result.output.ilName);
		TypeInstance u8Ref = u8TypeInstance;
		u8Ref.isRef = true;
		gen::Variable ref = gen::Variable{
			pointer, gen::ReferenceType::VALUE, u8Ref
		};
		returnValue(ILExprResultBuilder{}
			.withOutput(ref)
			.andInstructions(std::move(result.instructions))
			.buildAsPersistent());
		break;
	}
	default:
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
		if (!typeContext->getExactType<PrimitiveType>()) {
			throw SemanticError(expr.sourcePos, 
				fmt::format("Found an integer literal, when the following type was expected: {}", 
							typeContext->name));
		}
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
	IL::Program instructions;
	auto lhs = visitChild(expr.lhs);
	auto innerExpr = visitChild(expr.innerExpr);
	util::vector_append(instructions, std::move(lhs.instructions));
	util::vector_append(instructions, std::move(innerExpr.instructions));

	ListType const* list = expectListType(expr.sourcePos, lhs.output.type);
	gen::Variable elementRef = createBindingWithOffset(instructions, lhs.output, list->elementType, innerExpr.output);
	if (lhs.output.type.isMut) 
	{
		elementRef.type.isMut = true;
	}
	returnValue(ILExprResultBuilder{}
		.withOutput(elementRef)
		.andInstructions(std::move(instructions))
		.buildAsPersistent());
}

void ExprGenerator::visit(Expr::Cast const& expr)
{
	ILExprResult result = visitChild(expr.expr);
	TypeInstance castedType = env.types.instantiateType(expr.type);
	assertIsCastableType(expr.sourcePos, result.output.type, castedType);
	result.output = castVariable(result.instructions, result.output, castedType);
	returnValue(std::move(result));
}

void ExprGenerator::visit(Expr::Questionable const& expr)
{
	IL::Program instructions;
	auto arg = visitChild(expr.expr);
	if (!arg.output.type.isOpt) {
		throw SemanticError(expr.sourcePos, "Only a value with a maybe-type can be questioned");
	}
	gen::Variable result = getOptionalness(instructions, arg.output);
	returnValue(ILExprResultBuilder{}
			.withOutput(result)
			.andInstructions(std::move(instructions))
			.buildAsTemporary());
}

void ExprGenerator::visit(Expr::MemberAccess const& expr)
{
	ILExprResult lhs = visitChild(expr.lhs);
	BinType::Field const& member = expectMember(expr.sourcePos, lhs.output.type, expr.member);
	if (lhs.getReferenceType() != gen::ReferenceType::POINTER) 
	{
		throw SemanticError(expr.sourcePos, "Cannot perform member access on left hand side.");
	}
	gen::Variable offset = allocateConstant(lhs.instructions, member.offset, PrimitiveType::SubType::u16);
	gen::Variable memberBinding = createBindingWithOffset(lhs.instructions, lhs.output, member.type, offset);
	if (lhs.output.type.isMut)
	{
		memberBinding.type.isMut = true;
	}
	returnValue(ILExprResultBuilder{}
		.withOutput(memberBinding)
		.andInstructions(std::move(lhs.instructions))
		.buildAsTemporary(lhs.isTemporary()));
}

void ExprGenerator::visit(Expr::ListLiteral const& expr)
{
	IL::Program instructions;
	auto elements = util::transform_vector(expr.elements, [&](auto& element)
		{
			auto listType = typeContext->getExactType<ListType>();
			auto result = listType ? ExprGenerator::typedContext(env, listType->elementType.type).generate(element)
									: ExprGenerator::defaultContext(env).generate(element);
			util::vector_append(instructions, std::move(result.instructions));
			return result;
		});


	// default initialize the array
	ListType const* literalType = determineListLiteralType(expr.sourcePos, elements);
	gen::Variable buffer = allocateVariable(instructions, literalType);

	if (!elements.empty())
	{
		size_t factor = 1;
		for (size_t i = 0; i < elements.size(); ++i)
		{
			size_t stride = TargetInfo::calculateTypeSizeBytes(literalType->elementType);
			gen::Variable constant = allocateConstant(instructions, stride * i, PrimitiveType::SubType::u16);
			gen::Variable bufferElement = createBindingWithOffset(instructions, buffer, literalType->elementType, constant);
			assignVariable(instructions, bufferElement, elements[i].output);
		}
	}
	returnValue(ILExprResultBuilder{}
				.withOutput(buffer)
				.andInstructions(std::move(instructions))
				.buildAsTemporary());
}
void ExprGenerator::visit(Expr::StructLiteral const& expr)
{
	IL::Program instructions;
	auto binType = typeContext->getExactType<BinType>();
	if (!binType) {
		throw SemanticError(expr.sourcePos, "Unable to deduce the type of struct literal.");
	}
	if (expr.initializers.size() != binType->members.size()) {
		throw SemanticError(expr.sourcePos, "Provided an incorrect number of initializers in struct literal.");
	}
	gen::Variable storage = allocateVariable(instructions, TypeInstance(binType));

	for (size_t i = 0; i < binType->members.size(); ++i)
	{
		auto& member = binType->members[i];
		size_t initializerIdx = i;
		if (expr.names.has_value()) {
			auto& names = expr.names.value();
			auto it = std::find(names.begin(), names.end(), member.name);
			if (it == names.end()) {
				throw SemanticError(expr.sourcePos, fmt::format("Could not find member initializer for: {}", member.name));
			}
			initializerIdx = std::distance(names.begin(), it);
		}
		auto arg = ExprGenerator::typedContext(env, member.type.type).generate(expr.initializers[initializerIdx]);
		util::vector_append(instructions, std::move(arg.instructions));
		//assertIsAssignableType(expr.sourcePos, member.type, arg.output.type);
		gen::Variable offset = allocateConstant(instructions, member.offset, PrimitiveType::SubType::u16);
		gen::Variable memberBinding = createBindingWithOffset(instructions, storage, member.type, offset);
		assignVariable(instructions, memberBinding, arg.output);
	}
	returnValue(ILExprResultBuilder{}.withOutput(storage).andInstructions(std::move(instructions)).buildAsTemporary());
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