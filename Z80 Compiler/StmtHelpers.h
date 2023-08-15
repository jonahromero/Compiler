//DO NOT INCLUDE THIS
//just removed it from Stmt.h cause it was cluttered


struct Conditional {
	Conditional(Expr::UniquePtr expr)
		: expr(std::move(expr)) {}

	Expr::UniquePtr expr;
	StmtBody body;
};

struct Type {
	Type(std::string_view name, bool isMut)
		: name(name), isMut(isMut) {}

	std::string_view name;
	bool isMut;
};

//declarators: no expressions or anything fancy
struct VarDecl {
	VarDecl(std::string_view name, Type type)
		: name(name), type(type) {}

	std::string_view name;
	Type type;
};
struct TypeDecl {
	TypeDecl(std::string_view name)
		: name(name) {}
	std::string_view name;
};
struct TemplateDecl {
	void addDecl(VarDecl const& param) { params.push_back(param); }
	void addDecl(TypeDecl const& param) { params.push_back(param); }

	std::vector<std::variant<TypeDecl, VarDecl>> params;
};
