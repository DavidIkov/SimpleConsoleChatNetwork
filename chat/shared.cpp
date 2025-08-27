#include "shared.hpp"

namespace shared {
bool CheckUserNameSyntax(const char* name) {
    bool found_end = false;
    for (size_t i = 0; i < user_name_max_length && !found_end;
         found_end = name[i++]);
    return found_end;
}

bool CheckUserPasswordSyntax(const char* password) {
    bool found_end = false;
    for (size_t i = 0; i < user_password_max_length && !found_end;
         found_end = password[i++]);
    return found_end;
}

bool CheckRoomNameSyntax(const char* name) {
    bool found_end = false;
    for (size_t i = 0; i < room_name_max_length && !found_end;
         found_end = name[i++]);
    return found_end;
}

bool CheckRoomPasswordSyntax(const char* password) {
    bool found_end = false;
    for (size_t i = 0; i < room_password_max_length && !found_end;
         found_end = password[i++]);
    return found_end;
}

}  // namespace shared
