#pragma once
#include <cstdint>
#include<string_view>

namespace shared {

using id_t = int64_t;

static inline constexpr uint32_t user_name_max_length = 16;
static inline constexpr uint32_t user_password_max_length = 16;

[[nodiscard]] bool CheckUserNameSyntax(std::string_view name);
[[nodiscard]] bool CheckUserPasswordSyntax(std::string_view password);

static inline constexpr uint32_t room_name_max_length = 32;
static inline constexpr uint32_t room_password_max_length = 8;

[[nodiscard]] bool CheckRoomNameSyntax(std::string_view name);
[[nodiscard]] bool CheckRoomPasswordSyntax(std::string_view password);

}  // namespace shared
