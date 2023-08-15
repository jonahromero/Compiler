
#pragma once
#include "Stmt.h"
#include "CompilerError.h"

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
	enum class SubType { void_, bool_, u8, i8, u16, i16 } subtype;
	PrimitiveType(SubType subtype, std::string name, size_t size)
		: TypeWithId<PrimitiveType>(std::move(name), size), subtype(subtype) {}

	bool isUnsigned() const 
	{
		switch (subtype) 
		{
		case SubType::void_: return false;
		case SubType::bool_: return true;
		case SubType::u8: return false;
		case SubType::i8: return true;
		case SubType::u16: return false;
		case SubType::i16: return true;
		}
	}
	bool isLargestType() const
	{
		switch (subtype) 
		{
		case SubType::u16: return true;
		case SubType::i16: return true;
		default: return false;
		}
	}
	SubType getPromotedSubType() const 
	{
		switch (subtype)
		{
		case SubType::bool_: return SubType::i8;
		case SubType::u8: return SubType::i16;
		case SubType::i8: return SubType::i16;
		default: COMPILER_NOT_REACHABLE;
		}
	}
};

struct ArrayType final : TypeWithId<ArrayType>
{
	ArrayType(std::string name, size_t size, TypeInstance elementType, std::vector<size_t> indexing)
		: TypeWithId<ArrayType>(std::move(name), size), elementType(std::move(elementType)), indexing(std::move(indexing)) {}

	TypeInstance elementType;
	std::vector<size_t> indexing;
};

struct NoneType final : TypeWithId<NoneType>
{
	using TypeWithId<NoneType>::TypeWithId;
};

//templates 
struct TemplateBin final : TypeWithId<TemplateBin> 
{
	struct TypeParameter {};
	struct Parameter 
	{
		std::string_view name;
		std::variant<TypeParameter, TypeInstance> type;
	};

	TemplateBin(std::string name)
		: TypeWithId<TemplateBin>(std::move(name), 0) {}
	
	std::vector<Parameter> templateParams;
	std::vector<Stmt::VarDecl> body;
};

