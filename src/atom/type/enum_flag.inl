/*
 * enum_flag.inl
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-3

Description: Enum-based flags

**************************************************/

namespace Atom::Type {

template <typename TEnum>
inline void swap(Flags<TEnum>& flags1, Flags<TEnum>& flags2) noexcept {
    flags1.swap(flags2);
}

template <typename TEnum>
constexpr auto operator&(TEnum value1, TEnum value2) noexcept ->
    typename std::enable_if<IsEnumFlags<TEnum>::value, Flags<TEnum>>::type {
    return Flags<TEnum>(value1) & value2;
}

template <typename TEnum>
constexpr auto operator|(TEnum value1, TEnum value2) noexcept ->
    typename std::enable_if<IsEnumFlags<TEnum>::value, Flags<TEnum>>::type {
    return Flags<TEnum>(value1) | value2;
}

template <typename TEnum>
constexpr auto operator^(TEnum value1, TEnum value2) noexcept ->
    typename std::enable_if<IsEnumFlags<TEnum>::value, Flags<TEnum>>::type {
    return Flags<TEnum>(value1) ^ value2;
}

}  // namespace Atom::Type
