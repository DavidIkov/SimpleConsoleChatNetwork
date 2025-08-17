
#include "users.hpp"

#include <algorithm>
#include <cstring>

#include "client/user.hpp"

namespace server {

const UsersHandler::UserDB_Record* UsersHandler::GetUserFromDB_FromID(
    shared::user_id_t id) {
    std::lock_guard LG(mutex_);
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
    std::lock_guard LG(mutex_);
    auto iter =
        std::find_if(database_.begin(), database_.end(),
                     [username](UserDB_Record const& rec) {
                         return !std::strcmp(rec.username_, username);
                     });
    if (iter == database_.end())
        return nullptr;
    else
        return &*iter;
}
UsersHandler::UserAddingResult UsersHandler::AddUserToDB(const char* username,
                                                         const char* password) {
    std::lock_guard LG(mutex_);

    if (!shared::CheckUsernameSyntax(username))
        return {0, UserAddingResult::ResultType::IncorrectUsernameFormat};
    if (!shared::CheckPasswordSyntax(password))
        return {0, UserAddingResult::ResultType::IncorrectPasswordFormat};

    UserDB_Record& rec = database_.emplace_back();
    rec.id_ = ++id_counter_;
    std::strcpy(rec.username_, username);
    std::strcpy(rec.password_, password);
    return {rec.id_, UserAddingResult::ResultType::AddedInDB};
}

std::unique_ptr<client::ConnectionHandler> UsersHandler::_ConnectionFactory(
    EventsHandler::ClientRawDescriptor desc) {
    return std::make_unique<client::UserHandler>(this, desc);
}
}  // namespace server
