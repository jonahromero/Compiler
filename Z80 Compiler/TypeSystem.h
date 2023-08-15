#pragma once
#include <array>
#include <string>
#include "Stmt.h"
#include "Expr.h"
#include "CodeModule.h"
#include "TemplateSystem.h"

class TypeSystem
{

public:
	TypeSystem();

	struct TypeBase {
		TypeBase(std::string name, size_t size)
			: name(std::move(name)), size(size){}
		std::string name;
		size_t size;
	};
	using Type = std::unique_ptr<TypeBase>;

	struct BinType : TypeBase {
		struct Field {
			Type const& type;
			std::string_view name
		};
		std::vector<Field> innerTypes;
		bool mut;
	};
	struct FunctionType : TypeBase {
		std::vector<Type> params;
		Type retType;
	};
	struct PrimitiveType : TypeBase {
		using TypeBase::TypeBase;
	};
	struct Template : TypeBase {
		std::variant<Stmt::Function, Stmt::Bin> templateType;
	};

	struct TypeInstance {
		Type const& type;
		bool isMut;
	};

	void addFunc(Stmt::Function const& func);
	void addBin(Stmt::Bin const& bin);
	void addAlias(std::string_view name, TypeInstance instance);
	auto instantiateType(class Enviroment& env, Expr::UniquePtr& expr) -> TypeInstance;

	auto getType(std::string_view name)->Type const&;
	auto isType(std::string_view name) -> bool;

private:
	std::vector<Type> types;
};

