#pragma once
#include <cstddef>
#include <cstdint>
#include <ostream>

namespace shared {

static inline constexpr size_t user_name_max_length = 16;
static inline constexpr size_t user_password_max_length = 16;
using user_id_t = uint32_t;

[[nodiscard]] bool CheckUserNameSyntax(const char* name);
[[nodiscard]] bool CheckUserPasswordSyntax(const char* password);

struct User {
    user_id_t id_ = 0;
    char name_[user_name_max_length];
};

inline std::ostream& operator<<(std::ostream& stream, const User& user) {
    return stream << user.name_ << '[' << user.id_ << ']';
}

static inline constexpr size_t room_name_max_length = 32;
static inline constexpr size_t room_password_max_length = 8;
using room_id_t = uint32_t;

[[nodiscard]] bool CheckRoomNameSyntax(const char* name);
[[nodiscard]] bool CheckRoomPasswordSyntax(const char* password);

struct Room {
    room_id_t id_ = 0;
    char name_[room_name_max_length];
};

inline std::ostream& operator<<(std::ostream& stream, const Room& room) {
    return stream << room.name_ << '[' << room.id_ << ']';
}

}  // namespace shared
