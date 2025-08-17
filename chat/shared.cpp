#include "shared.hpp"

namespace shared {
bool CheckUsernameSyntax(const char* username) {
    bool found_end = false;
    for (size_t i = 0; i < username_max_length && !found_end;
         found_end = username[i++]);
    return found_end;
}

bool CheckPasswordSyntax(const char* password) {
    bool found_end = false;
    for (size_t i = 0; i < password_max_length && !found_end;
         found_end = password[i++]);
    return found_end;
}
}  // namespace shared
