#pragma once
#include <variant>

namespace util
{
    template<typename VariantType, typename T, size_t index = 0>
    constexpr size_t variantIndex() 
    {
        static_assert(std::variant_size_v<VariantType> > index, "Type not found in variant");
        if constexpr (index == std::variant_size_v<VariantType>) {
            return index;
        }
        else if constexpr (std::is_same_v<std::variant_alternative_t<index, VariantType>, T>) {
            return index;
        }
        else {
            return variantIndex<VariantType, T, index + 1>();
        }
    }

    template<class... Ts> struct OverloadVariant : Ts... { using Ts::operator()...; };
    template<class... Ts> OverloadVariant(Ts...) -> OverloadVariant<Ts...>;
}