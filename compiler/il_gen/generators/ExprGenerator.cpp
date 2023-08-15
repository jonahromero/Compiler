#include "ExprGenerator.h"
#include "VectorUtil.h"
#include "StringUtil.h"
#include "SemanticError.h"

ExprGenerator::ExprGenerator(Enviroment& env, IL::Type arithmeticType)
	: env(env), arithmeticType(arithmeticType)
{
}

ExprGenerator ExprGenerator::defaultContext(Enviroment& env)
{
	return ExprGenerator(env, IL::Type::i16);
}

ExprGenerator ExprGenerator::typedContext(Enviroment& env, IL::Type type)
{
	return ExprGenerator(env, type);
}

ILExprResult ExprGenerator::generateWithCast(Expr::UniquePtr& expr, IL::Type outputType)
{
	ILExprResult result = visitChild(expr);
	if (result.producesObjectRef()) {
		result.output = derefVariable(result.instructions, expr->sourcePos, result.output, result.getResultingRefType().type);
	}
	result.output = castVariable(result.instructions, expr->sourcePos, outputType, result.output);
	return result;
}

ILExprResult ExprGenerator::generate(Expr::UniquePtr& expr)
{
	return visitChild(expr);
}

void ExprGenerator::assertValidCast(SourcePosition pos, IL::Type from, IL::Type to)
{
	using enum IL::Type;
	if (to == i1) return;
	auto throwWithSuggestion = [&](const char* suggestion) {
		throw SemanticError(pos, fmt::format(
			"Unable to cast from {} to {}. {}", 
			IL::ilTypeToString(from), IL::ilTypeToString(to), suggestion)
		);
	};
	switch (from) {
	case i1: break;
	case u8: break;
	case i8: break;
	case u16:
		switch (to) {
		case u8: [[fallthrough]];
		case i8: [[fallthrough]];
		case u8_ptr: throwWithSuggestion("");
		case i16: throwWithSuggestion("Consider using u8 instead to avoid a potential overflow.");
		default:;
		}
	case i16:
		switch (to) {
		case u8: [[fallthrough]];
		case i8: [[fallthrough]];
		case u8_ptr: throwWithSuggestion("");
		default:;
		}
	case u8_ptr:
		throwWithSuggestion("");
	case void_:
		throwWithSuggestion("");
	}	
}

IL::Variable ExprGenerator::castVariable(IL::Program& instrs, SourcePosition pos, IL::Type newType, IL::Variable var)
{
	auto originalType = env.getILAliasType(var);
	if (originalType == newType) return var;
	assertValidCast(pos, originalType, newType);
	auto castedVariable = env.createAnonymousVariable(newType);
	instrs.emplace_back(IL::makeIL<IL::Cast>(castedVariable, newType, var));
	return castedVariable;
}

void ExprGenerator::assertValidArgTypePassedToFunction(SourcePosition pos, TypeInstance param, TypeInstance arg) const
{

}

IL::Variable ExprGenerator::derefVariable(IL::Program& instructions, IL::Variable var, PrimitiveType const* refType, bool isOpt)
{
	COMPILER_ASSERT("Cannot dereference a non-pointer type", env.getILAliasType(var) == IL::Type::u8_ptr);
	if (isOpt) {
		IL::Variable lhs = var, rhs = env.createAnonymousVariable(IL::Type::u16);
		var = env.createAnonymousVariable();
		instructions.push_back(IL::makeIL<IL::Assignment>(rhs, IL::Type::u16, int(1)));
		instructions.push_back(IL::makeIL<IL::Binary>(var, IL::Type::u8_ptr, lhs, Token::Type::PLUS, rhs));
	}
	IL::Type instantiatedType = env.types.compileType(TypeInstance(refType));
	IL::Variable derefResult = env.createAnonymousVariable(instantiatedType);
	instructions.push_back(IL::makeIL<IL::Deref>(derefResult, instantiatedType, var));
	return derefResult;
}

FunctionType const& ExprGenerator::evaluateToCallable(Expr::UniquePtr expr)
{
	auto throwNonCallable = [&]() {throw SemanticError(expr.sourcePos, "Expression does not result in a callable type."); };
	auto compiledExpr = visitChild(expr);
}

std::pair<IL::Type, IL::Type> ExprGenerator::generateBinaryTypeExprResult(SourcePosition pos, IL::Type leftType, Token::Type oper, IL::Type rightType)
{
	IL::Type leftCast, rightCast;

	if (isLogicalOperator(oper)) {
		leftCast = rightCast = IL::Type::i1;
	}
	else {
		bool isLeftPtr = IL::isIlTypePointer(leftType);
		bool isRightPtr = IL::isIlTypePointer(rightType);
		if (isLeftPtr && isRightPtr) {
			throw SemanticError(pos, "Unable to add two pointers together. Please provide explicit cast.");
		}
		if (isLeftPtr) {
			leftCast = leftType;
			rightCast = IL::Type::u16;
			returnType = IL::Type::u8_ptr;
		}
		else if (isRightPtr) {
			rightCast = rightType;
			leftCast = IL::Type::u16;
			returnType = IL::Type::u8_ptr;
		}
		else {
			if (leftType == rightType) {
				leftCast = leftType;
				rightCast = rightType;
			}
			else {
				auto leftSize = IL::ilTypeBitSize(leftType);
				auto rightSize = IL::ilTypeBitSize(rightType);
				if (leftSize == rightSize) {
					bool preferenceIsUnsigned = IL::isIlTypeUnsigned(arithmeticType);
					if (preferenceIsUnsigned) {
						leftCast = IL::ilTypeAsUnsigned(leftType);
						rightCast = IL::ilTypeAsUnsigned(rightType);
					}
					else {
						leftCast = IL::ilTypeAsSigned(leftType);
						rightCast = IL::ilTypeAsSigned(rightType);
					}
				}
				else {
					auto largerType = leftSize > rightSize ? leftType : rightType;
					leftCast = largerType;
					rightCast = largerType;
				}
			}
			returnType = leftCast;
		}
	}
	return BinaryTypeExprResult{ returnType, leftCast, rightCast };
}

void ExprGenerator::visit(Expr::Binary& expr)
{
	IL::ILBody instructions;
	auto lhs = visitChild(expr.lhs), rhs = visitChild(expr.rhs);
	util::vector_append(instructions, std::move(lhs.instructions));
	util::vector_append(instructions, std::move(rhs.instructions));

	if (lhs.isPointerReferenceType()) lhs.output = derefVariable(instructions, expr.sourcePos, lhs.output, lhs.type);
	if (rhs.isPointerReferenceType()) rhs.output = derefVariable(instructions, expr.sourcePos, rhs.output, rhs.type);

	auto exprType = performBinaryOperationWithTypes(expr.sourcePos, lhs.type, rhs.type);
	auto [leftCast, rightCast] = generateBinaryCasts(expr.sourcePos, env.getILAliasType(lhs.output), expr.oper, env.getILAliasType(rhs.output));

	lhs.output = castVariable(instructions, expr.sourcePos, env.types.compileType(leftCast), lhs.output);
	rhs.output = castVariable(instructions, expr.sourcePos, env.types.compileType(rightCast), rhs.output);

	auto binaryOutput = createNewILVariable();
	instructions.emplace_back(IL::makeIL<IL::Binary>(binaryOutput, env.types.compileType(exprType), lhs.output, expr.oper, rhs.output));
	
	returnValue(ILExprResultBuilder{}.createTemporary()
									 .withOutputAt(binary).withExprType(exprType)
									 .andInstructions(std::move(instructions))
									 .build());
}

void ExprGenerator::visit(Expr::Unary& expr)
{
	IL::Type type = isRelationalOperator(expr.oper) || isLogicalOperator(expr.oper) ? IL::Type::i1 : arithmeticType;
	ILExprResult result = newResult(type);
	auto rhs = visitChild(expr.expr);
	util::vector_append(result.instructions, std::move(rhs.instructions));
	auto rhsVar = rhs.output;
	if (isLogicalOperator(expr.oper)) {
		rhsVar = castVariable(result.instructions, expr.sourcePos, IL::Type::i1, rhs.output);
	}
	result.instructions.emplace_back(IL::makeIL<IL::Unary>(result.output, type, expr.oper, rhsVar));
	returnValue(std::move(result));
}

IL::Variable ExprGenerator::createNewILVariable(IL::Type type) const
{
	return env.createAnonymousVariable(type);
}

void ExprGenerator::visit(Expr::Parenthesis& expr)
{
	returnValue(visitChild(expr.expr));
}

void ExprGenerator::visit(Expr::Identifier& expr)
{
	if (env.isValidVariable(expr.ident)) {
		TypeInstance type = env.getVariableType(expr.ident);
		ILExprResult result = ILExprResult{ env.getVariableILAlias(expr.ident), type };
		//result.output = castVariable(result.instructions, expr.sourcePos, arithmeticType, result.output);
		returnValue(std::move(result));
	}
	else {
		throw SemanticError(expr.sourcePos, util::strBuilder("Use of unknown variable: ", expr.ident));
	}
}

void ExprGenerator::visit(Expr::Literal& expr)
{
	std::visit([&](auto&& arg) 
	{
		using U = std::remove_cvref_t<decltype(arg)>;
		if constexpr (std::is_same_v<U, std::string>) {
			ILExprResult result = newResult(IL::Type::u8_ptr);
			COMPILER_NOT_SUPPORTED;
			result.instructions.emplace_back(
				IL::makeIL<IL::Assignment>(result.output, IL::Type::u8_ptr, std::move(arg))
			);
			returnValue(std::move(result));
		}
		else if constexpr (std::is_same_v<U, u16>) {
			ILExprResult result = newResult(arithmeticType);
			result.instructions.emplace_back(
				IL::makeIL<IL::Assignment>(result.output, arithmeticType, std::move(arg))
			);
			returnValue(std::move(result));
		}
		else if constexpr (std::is_same_v<U, std::monostate>) {
			COMPILER_NOT_REACHABLE;
		}
	}, expr.literal);
}

// Need to finish
void ExprGenerator::visit(Expr::FunctionCall& expr) 
{
	auto throwNonCallable = [&]() {throw SemanticError(expr.sourcePos, "Expression does not result in a callable type."); };
	auto funcPtr = visitChild(expr.lhs);

	TypeInstance refType = funcPtr.getResultingRefType();
	FunctionType* funcType = refType.type->getExactType<FunctionType>();
	if (!funcType) throwNonCallable();
	if (funcType->params.size() != expr.arguments.size()) {
		throw SemanticError(expr.sourcePos, 
			fmt::format("Function expected {} arguments, however, it recieved {} instead.", 
			funcType->params.size(), expr.arguments.size()
		));
	}
	auto argResults = util::transform_vector(funcType->params, expr.arguments, [&](TypeInstance const& param, Expr::UniquePtr const& arg) {
		auto argResult = visitChild(arg);
		assertValidArgTypePassedToFunction(arg->sourcePos, param, argResult.outputType);
		return argResult;
	});
	if(funcType->returnType)
	if (funcPtr.isNamed()) {
			
	}
	else {
			
	}
}

void ExprGenerator::assertCanPerformArithmeticWith(SourcePosition const& pos, TypeInstance const& type)
{
	auto primitiveType = type.type->getExactType<PrimitiveType>();
	if (!primitiveType)
	{
		const char* msg = "Unable to perform the operation "
						  "requested with one of the operands with type: {}";
		throw SemanticError(fmt::format(msg, lhs.type.type->name));
	}
	else if (type.isOpt) 
	{
	
	}
}


TypeInstance ExprGenerator::performBinaryOperationWithTypes(SourcePosition pos, TypeInstance lhs, Token::Type oper, TypeInstance rhs) const
{
	assertCanPerformArithmeticWith(pos, lhs);
	assertCanPerformArithmeticWith(pos, rhs);
	if (lhs.type != rhs.type) 
	{
		std::string msg = fmt::format("Unable to perform a binary operation on two incompatible "
									  "types: {}, and {}", lhs.type->name, rhs.type->name);
		throw SemanticError(pos, std::move(msg));
	}
	if (isLogicalOperator(oper) || isRelationalOperator(oper)) {
		return TypeInstance(env.types.getType("bool"));
	}
	else {
		return TypeInstance(lhs.type);
	}
}

void ExprGenerator::visit(Expr::Indexing& expr) {
	
}

void ExprGenerator::visit(Expr::Cast& expr)
{

}

void ExprGenerator::visit(Expr::Questionable& expr)
{
	auto evaluated = visitChild(expr.expr);
	if (!evaluated.producesObjectRef()) {
		throw;
	}
}

void ExprGenerator::visit(Expr::MemberAccess& expr)
{
	ILExprResult lhs = visitChild(expr.lhs);
	if (!lhs.producesObjectRef()) {
		throw SemanticError(expr.sourcePos, "Left hand side of member access must be a valid l-value");
	}
	TypeInstance type = lhs.getResultingRefType();
	auto* bin = type.type->getExactType<BinType>();
	if (!bin) {
		throw SemanticError(expr.sourcePos, fmt::format("Cannot perform member access on the following type: {}", type.type->name));
	}
	auto it = std::find_if(bin->members.begin(), bin->members.end(), [member = expr.member](auto& field) {
		return member == field.name;
	});
	if (it == bin->members.end()) {
		throw SemanticError(expr.sourcePos, fmt::format("No member \"{}\" exists in the type: {}", expr.member, bin->name));
	}
	size_t offset = it->offset;
	TypeInstance memberType = it->type;
	ILExprResult result = newRefResult(IL::Type::u8_ptr, memberType);
	util::vector_append(result.instructions, std::move(lhs.instructions));
	auto offsetVariable = env.createAnonymousVariable(IL::Type::u16);
	result.instructions.push_back(IL::makeIL<IL::Assignment>(offsetVariable, IL::Type::u16, int(offset)));
	result.instructions.push_back(IL::makeIL<IL::Binary>(result.output, IL::Type::u8_ptr, lhs.output, Token::Type::PLUS, offsetVariable));
	returnValue(std::move(result));
}

void ExprGenerator::visit(Expr::ListLiteral& expr) 
{
	returnValue(newResult(IL::Type::u8_ptr));
}
void ExprGenerator::visit(Expr::StructLiteral& expr) 
{
	returnValue(newResult(IL::Type::u8_ptr));
}

bool ExprGenerator::isPrimitiveType(TypePtr type)
{
	return type->getExactType<PrimitiveType>();
}

// This is somewhat non sensical. Nothing to compile this to.
void ExprGenerator::visit(Expr::TemplateCall& expr) {
	COMPILER_NOT_SUPPORTED;
}
void ExprGenerator::visit(Expr::Reference& expr) {
	COMPILER_NOT_SUPPORTED;
}

// These are only allowed within instructions.
void ExprGenerator::visit(Expr::Register& expr) {}
void ExprGenerator::visit(Expr::Flag& expr) {}
void ExprGenerator::visit(Expr::CurrentPC& expr) {}


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
		EQUAL_EQUAL,NOT_EQUAL, LESS, LESS_EQUAL,
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