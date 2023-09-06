
#include "ComputedExpr.h"
#include "StringUtil.h"

ComputedExpr::ComputedExpr(SourcePosition sourcePos, VarType const& value) : sourcePos(sourcePos), value(value) {}
ComputedExpr::ComputedExpr(SourcePosition sourcePos, VarType&& value) : sourcePos(sourcePos), value(std::move(value)) {}

std::string_view ComputedExpr::typeToString() const
{
	return std::visit([&](auto&& arg) {
		using U = std::remove_cvref_t<decltype(arg)>;
		if constexpr (std::is_same_v<U, u16>) {
			return "integer";
		}
		else if constexpr (std::is_same_v<U, std::string>) {
			return "string";
		}
		else {
			return "type";
		}
	}, value);
}

std::string ComputedExpr::toString() const
{
	return std::visit([&](auto&& arg) -> std::string {
		using U = std::remove_cvref_t<decltype(arg)>;
		if constexpr (std::is_same_v<U, u16>) {
			return std::to_string(arg);
		}
		else if constexpr (std::is_same_v<U, std::string>) {
			return util::strBuilder('\"', arg, '\"');
		}
		else {
			return arg.type->name;
		}
	}, value);
}

Expr::UniquePtr ComputedExpr::toExpr() const
{
	return std::visit([&](auto&& arg) {
		using U = std::remove_cvref_t<decltype(arg)>;
		if constexpr (std::is_same_v<U, u16> || std::is_same_v<U, std::string>) {
			return Expr::makeExpr<Expr::Literal>(sourcePos, std::move(arg));
		}
		else {
			return Expr::makeExpr<Expr::Identifier>(sourcePos, arg.type->name);
		}
	}, value);
}

bool ComputedExpr::isTypeInstance() const
{
	return std::holds_alternative<TypeInstance>(value);
}

bool ComputedExpr::isInt() const
{
	return std::holds_alternative<u16>(value);
}

bool ComputedExpr::isString() const
{
	return std::holds_alternative<std::string>(value);
}

TypeInstance const& ComputedExpr::getTypeInstance() const
{
	return std::get<TypeInstance>(value);
}

std::string const& ComputedExpr::getString() const
{
	return std::get<std::string>(value);
}

u16 ComputedExpr::getInt() const
{
	return std::get<u16>(value);
}
