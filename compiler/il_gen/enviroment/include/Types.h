
#pragma once
#include "Stmt.h"

class Type 
{
public:
	Type(std::string name, size_t size)
		: name(std::move(name)), size(size) {}
	virtual ~Type() = default;

	template<typename T>
	T const* getExactType() const 
	{
		return this->id() == typeid(T).hash_code() ? static_cast<T const*>(this) : nullptr;
	}

	std::string name;
	size_t size;
private:
	virtual size_t id() const = 0;
};

template<typename T>
class TypeWithId : public Type
{
public:
	using Type::Type;
private:
	virtual size_t id() const override {
		return typeid(T).hash_code(); 
	}
};

using TypePtr = Type const*;

struct TypeInstance
{
	TypeInstance(TypePtr type) : type(type) {
		isMut = isRef = isOpt = false;
	}
	TypePtr type;
	bool isMut, isRef, isOpt;
};

struct BinType final : TypeWithId<BinType>
{
	struct Field 
	{
		TypeInstance type;
		std::string_view name;
		size_t offset;
	};

	using TypeWithId<BinType>::TypeWithId;
	std::vector<Field> members;
};

struct FunctionType final : TypeWithId<FunctionType>
{
	FunctionType(std::string name, size_t size, std::vector<TypeInstance> paramTypes, TypeInstance returnType)
		: TypeWithId<FunctionType>(std::move(name), size), params(std::move(paramTypes)), returnType(returnType) {}

	std::vector<TypeInstance> params;
	TypeInstance returnType;
};

struct PrimitiveType final : TypeWithId<PrimitiveType> 
{
	using TypeWithId<PrimitiveType>::TypeWithId;
};

struct NoneType final : TypeWithId<NoneType>
{
	using TypeWithId<PrimitiveType>::TypeWithId;
};

//templates 
struct TemplateBin final : TypeWithId<TemplateBin> 
{
	struct TypeParam {};
	struct TemplateParam {
		std::string_view name;
		std::variant<TypeParam, TypeInstance> type;
	};

	TemplateBin(std::string name)
		: TypeWithId<TemplateBin>(std::move(name), 0) {}
	
	std::vector<TemplateParam> templateParams;
	std::vector<Stmt::VarDecl> body;
};

