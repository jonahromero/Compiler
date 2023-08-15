#pragma once
#include "Expr.h"
#include <variant>
#include <string>
#include "IntTypes.h"
#include "Types.h"

class ComputedExpr 
{
	using VarType = std::variant<std::string, u16, TypeInstance>;
public:
	ComputedExpr(SourcePosition sourcePos, VarType const& value);
	ComputedExpr(SourcePosition sourcePos, VarType&& value);

	std::string_view typeToString() const;
	std::string toString() const;
	Expr::UniquePtr toExpr() const;

	bool isTypeInstance() const;
	bool isInt() const;
	bool isString() const;

	TypeInstance const& getTypeInstance() const;
	std::string const& getString() const;
	u16 getInt() const;

	SourcePosition sourcePos;
private:
	VarType value;
};