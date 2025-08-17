#pragma once
#include <cstddef>
#include <cstdint>

namespace shared {

static inline constexpr size_t username_max_length = 16;
static inline constexpr size_t password_max_length = 16;
using user_id_t = uint32_t;

[[nodiscard]] bool CheckUsernameSyntax(const char* username);
[[nodiscard]] bool CheckPasswordSyntax(const char* password);

}  // namespace shared
