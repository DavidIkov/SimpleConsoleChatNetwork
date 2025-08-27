#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <variant>

#include "shared.hpp"

namespace events {

using EnumType = uint32_t;
static_assert(sizeof(EnumType) <= sizeof(size_t), "enum type size is too big");

enum class Type : EnumType {
    HelloWorld,
    LoginAttemp,
    LoginAttempRespond,
    Logout,
    JoinRoomAttemp,
    JoinRoomAttempRespond,
    LeaveRoom,
    END_OF_ENUM
};

template <Type>
struct StructWrapper;

template <>
struct StructWrapper<Type::HelloWorld> {
    unsigned counter_;
};

using HelloWorldEvent = StructWrapper<Type::HelloWorld>;

template <>
struct StructWrapper<Type::LoginAttemp> {
    char name_[shared::user_name_max_length];
    char password_[shared::user_password_max_length];
};
using LoginAttempEvent = StructWrapper<Type::LoginAttemp>;

template <>
struct StructWrapper<Type::LoginAttempRespond> {
    enum class RespondType : uint8_t {
        LoggedIn,
        AlreadyLoggedIn,
        IncorrectNameFormat,
        IncorrectPasswordFormat,
        WrongPassword,
        RegisteredAsNewUser,
        Unknown,
    } response_;
    shared::user_id_t id_;
};
using LoginAttempRespondEvent = StructWrapper<Type::LoginAttempRespond>;

template <>
struct StructWrapper<Type::Logout> {};
using LogoutEvent = StructWrapper<Type::Logout>;

template <>
struct StructWrapper<Type::JoinRoomAttemp> {
    char name_[shared::room_name_max_length];
    char password_[shared::room_password_max_length];
};
using JoinRoomAttempEvent = StructWrapper<Type::JoinRoomAttemp>;

template <>
struct StructWrapper<Type::JoinRoomAttempRespond> {
    enum class RespondType : uint8_t {
        Joined,
        NotLoggedIn,
        AlreadyInRoom,
        IncorrectNameFormat,
        IncorrectPasswordFormat,
        WrongPassword,
        JoinedNewRoom,
        Unknown,
    } response_;
    shared::room_id_t id_;
};
using JoinRoomAttempRespondEvent = StructWrapper<Type::JoinRoomAttempRespond>;

template <>
struct StructWrapper<Type::LeaveRoom> {};
using LeaveRoomEvent = StructWrapper<Type::LeaveRoom>;

// some usefull stuff down here

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
constexpr auto _GetAllEventVariantHelper(
    std::integer_sequence<EnumType, typ...>) {
    return std::variant<StructWrapper<(Type)typ>...>();
}

using AllEventsVariant = decltype(_GetAllEventVariantHelper(
    std::make_integer_sequence<EnumType, (EnumType)Type::END_OF_ENUM>()));

}  // namespace events
