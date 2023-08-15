//DO NOT INCLUDE THIS
//just removed it from Stmt.h cause it was cluttered


struct Conditional {
	Conditional() = default;
	Conditional(Expr::UniquePtr expr)
		: expr(std::move(expr)) {}
	Conditional(Expr::UniquePtr expr, StmtBody body)
		: expr(std::move(expr)), body(std::move(body)) {}
	auto deepCopy() const { return Conditional(expr->clone(), copyStmtBody(body)); }

	::Expr::UniquePtr expr;
	StmtBody body;
};

// this is a type decl like: "MyType : type"
struct TypeDecl {
	TypeDecl(std::string_view name)
		: name(name) {}
	auto deepCopy() const { return TypeDecl(name); }

	std::string_view name;
};

//declarators: no expressions or anything fancy
struct VarDecl {
	VarDecl(std::string_view name, ::Expr::UniquePtr type)
		: name(name), type(std::move(type)) {}
	auto deepCopy() const { return VarDecl(name, type->clone()); }
	std::string_view name;
	::Expr::UniquePtr type;
};

auto copyVarDeclList(std::vector<VarDecl> const& varDecls) {
	std::vector<VarDecl> copiedVarDecls; copiedVarDecls.reserve(varDecls.size());
	for (auto& varDecl : varDecls) copiedVarDecls.push_back(varDecl.deepCopy());
	return copiedVarDecls;
}

using GenericDecl = std::variant<VarDecl, TypeDecl>;

auto copyGenericDecl(GenericDecl const& decl) -> GenericDecl {
	if (std::holds_alternative<VarDecl>(decl)) {
		return std::get<VarDecl>(decl).deepCopy();
	}
	else {
		return std::get<TypeDecl>(decl).deepCopy();
	}
}

struct TemplateDecl {
	TemplateDecl(std::vector<GenericDecl> params) : params(std::move(params)) {}

	std::vector<GenericDecl> params;
	
	auto deepCopy() const {
		std::vector<GenericDecl> copiedParams; copiedParams.reserve(params.size());
		for (auto& param : params) copiedParams.push_back(copyGenericDecl(param));
		return TemplateDecl{ std::move(copiedParams) };
	}
};
