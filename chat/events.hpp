#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <variant>

namespace Events {

using EnumType = uint32_t;
static_assert(sizeof(EnumType) <= sizeof(size_t), "enum type size is too big");

enum Type : EnumType { HelloWorld, END_OF_ENUM };

template <Type>
struct StructWrapper;

template <>
struct StructWrapper<HelloWorld> {
    int num;
};

template <EnumType typ, EnumType... typs>
constexpr size_t _GetMaxSizeOfEvent(
    std::integer_sequence<EnumType, typ, typs...>) {
    if constexpr (sizeof...(typs))
        return std::max(
            sizeof(StructWrapper<(Type)typ>),
            _GetMaxSizeOfEvent(std::integer_sequence<EnumType, typs...>()));
    else
        return sizeof(StructWrapper<(Type)typ>);
}

constexpr inline size_t MaxSizeOfEvent = _GetMaxSizeOfEvent(
    std::make_integer_sequence<EnumType, (EnumType)Type::END_OF_ENUM>{});

constexpr inline size_t MaxSizeOfPacket = sizeof(EnumType) + MaxSizeOfEvent;

template <size_t ind, size_t... inds>
constexpr void _FillSizesOfEvents(
    std::array<size_t, (size_t)Type::END_OF_ENUM>& arr,
    std::integer_sequence<size_t, ind, inds...>) {
    arr[ind] = sizeof(StructWrapper<(Type)ind>);
    if constexpr (sizeof...(inds))
        _FillSizesOfEvents(arr, std::integer_sequence<size_t, inds...>());
}
constexpr auto _CreateSizesOfEvents() {
    std::array<size_t, (size_t)Type::END_OF_ENUM> arr{};
    _FillSizesOfEvents(arr, std::make_integer_sequence<size_t, arr.size()>());
    return arr;
}

constexpr inline auto SizesOfEvents = _CreateSizesOfEvents();

template <EnumType... typ>
constexpr auto _GetAllEventVariantHelper(std::integer_sequence<EnumType, typ...>) {
    return std::variant<StructWrapper<(Type)typ>...>();
}

using AllEventsVariant = decltype(_GetAllEventVariantHelper(std::make_integer_sequence<EnumType, (EnumType)Type::END_OF_ENUM>()));

}  // namespace Events
