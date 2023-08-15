#pragma once
#include <array>
#include <string>
#include "Stmt.h"
#include "Expr.h"
#include "CodeModule.h"

class TypeSystem
{
public:
	TypeSystem(class Enviroment& env);

	struct Type {
		Type(std::string name, size_t size)
			: name(std::move(name)), size(size){}
		std::string name;
		size_t size;
	};
	using TypePtr = Type const*;

	struct BinType : Type {
		struct Field {
			TypePtr type;
			std::string_view name;
		};
		using Type::Type;
		std::vector<Field> innerTypes;
	};
	struct FunctionType : Type {
		using Type::Type;
		std::vector<TypePtr> params;
		TypePtr retType;
	};
	struct PrimitiveType : Type {
		using Type::Type;
	};

	//templates 
	template<typename StmtType>
	struct Template : Type {
		Template(std::string name, StmtType type, size_t size) 
			: Type(std::move(name), size), type(std::move(type)) {}
		StmtType type;
	};

	struct TypeInstance {
		size_t size() const { return type->size; }
		TypePtr type;
		bool isMut;
	};

	void addFunc(Stmt::Function& func);
	void addBin(Stmt::Bin& bin);
	void addAlias(std::string_view name, TypeInstance instance);
	auto instantiateType(Expr::UniquePtr& expr) -> TypeInstance;

	auto getType(std::string_view name)->Type const&;
	auto isType(std::string_view name) -> bool;

private:
	std::vector<std::unique_ptr<Type>> types;
	std::unordered_map<std::string_view, std::string_view> aliases;
	class Enviroment& env;
};

