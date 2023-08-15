#include <string>

namespace IL 
{
	enum class Type {
		i1, i8, i16, u8, u16, u8_ptr, void_
	};

	inline std::string_view ilTypeToString(Type type) {
		switch (type) {
		case Type::i1: return "i1";
		case Type::u8: return "u8";
		case Type::i8: return "i8";
		case Type::u16: return "u16";
		case Type::i16: return "i16";
		case Type::u8_ptr: return "u8*";
		case Type::void_: return "void";
		}
	}
	inline size_t ilTypeBitSize(Type type) {
		switch (type) {
		case Type::i1: return 1;
		case Type::u8: return 8;
		case Type::i8: return 8;
		case Type::u16: return 16;
		case Type::i16: return 16;
		case Type::u8_ptr: return 16;
		case Type::void_: return 0;
		}
	}
	inline bool isIlTypePointer(Type type) {
		switch (type) {
		case Type::u8_ptr: return true;
		default: return false;
		}
	}
	inline bool isIlTypeUnsigned(Type type) 
	{
		switch (type) {
		case Type::i1: return false;
		case Type::u8: return true;
		case Type::i8: return false;
		case Type::u16: return true;
		case Type::i16: return false;
		case Type::u8_ptr: return true;
		case Type::void_: return false;
		}
	}
	inline Type ilTypeAsUnsigned(Type type)
	{
		switch (type) {
		case Type::i1: return Type::i1;
		case Type::u8: return Type::u8;
		case Type::i8: return Type::u8;
		case Type::u16: return Type::u16;
		case Type::i16: return Type::u16;
		case Type::u8_ptr: return Type::u8_ptr;
		case Type::void_: return Type::void_;
		}
	}
	inline Type ilTypeAsSigned(Type type)
	{
		switch (type) {
		case Type::i1: return Type::i1;
		case Type::u8: return Type::i8;
		case Type::i8: return Type::i8;
		case Type::u16: return Type::i16;
		case Type::i16: return Type::i16;
		case Type::u8_ptr: return Type::u8_ptr;
		case Type::void_: return Type::void_;
		}
	}
}