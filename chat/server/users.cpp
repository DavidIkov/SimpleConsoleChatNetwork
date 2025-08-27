
#include "users.hpp"

#include <algorithm>
#include <cstring>

#include "client/user.hpp"

namespace server {

const UsersHandler::UserDB_Record* UsersHandler::GetUserFromDB_FromID(
    shared::user_id_t id) {
    auto iter =
        std::find_if(database_.begin(), database_.end(),
                     [id](UserDB_Record const& rec) { return rec.id_ == id; });
    if (iter == database_.end())
        return nullptr;
    else
        return &*iter;
}
const UsersHandler::UserDB_Record* UsersHandler::GetUserFromDB_FromUsername(
    const char* username) {
    auto iter = std::find_if(database_.begin(), database_.end(),
                             [username](UserDB_Record const& rec) {
                                 return !std::strcmp(rec.name_, username);
                             });
    if (iter == database_.end())
        return nullptr;
    else
        return &*iter;
}
UsersHandler::UserAddingResult UsersHandler::AddUserToDB(const char* username,
                                                         const char* password) {
    if (!shared::CheckUserNameSyntax(username))
        return {0, UserAddingResult::ResultType::IncorrectUsernameFormat};
    if (!shared::CheckUserPasswordSyntax(password))
        return {0, UserAddingResult::ResultType::IncorrectPasswordFormat};

    UserDB_Record& rec = database_.emplace_back();
    rec.id_ = ++id_counter_;
    std::strcpy(rec.name_, username);
    std::strcpy(rec.password_, password);
    return {rec.id_, UserAddingResult::ResultType::AddedInDB};
}

std::unique_ptr<client::Base> UsersHandler::_ConnectionFactory(
    EventsHandler::ClientRawDescriptor desc) {
    return std::make_unique<client::UserHandler>(this, desc);
}
}  // namespace server
