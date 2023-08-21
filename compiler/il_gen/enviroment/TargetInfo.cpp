
#include "TargetInfo.h"

size_t TargetInfo::calculateTypeSizeBytes(TypeInstance type)
{
	size_t sizeInBits = calculateTypeSizeBits(type);
	return sizeInBits == 1 ? 1 : sizeInBits / 8;
}

size_t TargetInfo::calculateTypeSizeBits(TypeInstance type)
{
	if (auto primitiveType = type.type->getExactType<PrimitiveType>())
	{
		if (primitiveType->subtype == PrimitiveType::SubType::bool_
			&& !type.isRef && !type.isOpt)
			return 1;
	}
	size_t size = type.isRef ? POINTER_SIZE_BITS : (type.type->size * 8);
	return size + (type.isOpt ? 1 : 0);
}

size_t TargetInfo::getPointerSizeBytes()
{
	return POINTER_SIZE_BITS / 2;
}

bool TargetInfo::fitsInRegister(TypeInstance type)
{
	return calculateTypeSizeBits(type) <= LARGEST_REGISTER_SIZE_BITS;
}

bool TargetInfo::isUnsignedType(TypeInstance type)
{
	if (type.isOpt || type.isOpt) return true;
	if (auto primitiveType = type.type->getExactType<PrimitiveType>())
	{
		return primitiveType->isUnsigned();
	}
	else {
		return true;
	}
}
