
#pragma once
#include "Types.h"

class TargetInfo 
{
public:
	static size_t calculateTypeSizeBytes(TypeInstance type);
	static size_t calculateTypeSizeBits(TypeInstance type);
	static size_t getPointerSizeBytes();

	static bool fitsInRegister(TypeInstance type);
	static bool isUnsignedType(TypeInstance type);
private:
	// The sizes and implementation functions allow for hard coding some 
	// implementation and compiler specifics
	static constexpr size_t POINTER_SIZE_BITS = 16, LARGEST_REGISTER_SIZE_BITS = 16;
};