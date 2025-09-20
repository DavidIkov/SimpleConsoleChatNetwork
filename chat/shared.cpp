#include "shared.hpp"

#include <cstddef>

namespace shared {
bool CheckUserNameSyntax(std::string_view name) {
    bool found_end = false;
    for (size_t i = 0; i < user_name_max_length && !found_end;
         found_end = name[i++]);
    return found_end;
}

bool CheckUserPasswordSyntax(std::string_view password) {
    bool found_end = false;
    for (size_t i = 0; i < user_password_max_length && !found_end;
         found_end = password[i++]);
    return found_end;
}

bool CheckRoomNameSyntax(std::string_view name) {
    bool found_end = false;
    for (size_t i = 0; i < room_name_max_length && !found_end;
         found_end = name[i++]);
    return found_end;
}

bool CheckRoomPasswordSyntax(std::string_view password) {
    bool found_end = false;
    for (size_t i = 0; i < room_password_max_length && !found_end;
         found_end = password[i++]);
    return found_end;
}

}  // namespace shared
