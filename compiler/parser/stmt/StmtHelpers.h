
struct Conditional {
	Conditional() = default;
	Conditional(Expr::UniquePtr expr)
		: expr(std::move(expr)) {}
	Conditional(Expr::UniquePtr expr, StmtBody body)
		: expr(std::move(expr)), body(std::move(body)) {}

	::Expr::UniquePtr expr;
	StmtBody body;
};

// this is a type decl like: "MyType : type"
struct TypeDecl {
	TypeDecl(std::string_view name)
		: name(name) {}

	std::string_view name;
};

//declarators: no expressions or anything fancy
struct VarDecl {
	VarDecl(std::string_view name, ::Expr::UniquePtr type)
		: name(name), type(std::move(type)) {}
		
	std::string_view name;
	::Expr::UniquePtr type;
};

using GenericDecl = std::variant<VarDecl, TypeDecl>;

struct TemplateDecl {
	TemplateDecl() = default;
	TemplateDecl(std::vector<GenericDecl> params) : params(std::move(params)) {}

	std::vector<GenericDecl> params;
};
