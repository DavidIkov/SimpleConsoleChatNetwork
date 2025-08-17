#pragma once
#include "connections.hpp"

namespace server {
class UsersHandler : public ConnectionsHandler {
public:
    struct UserDB_Record {
        shared::user_id_t id_;
        char username_[shared::username_max_length];
        char password_[shared::password_max_length];
    };

    [[nodiscard]] const UserDB_Record* GetUserFromDB_FromID(
        shared::user_id_t id);
    [[nodiscard]] const UserDB_Record* GetUserFromDB_FromUsername(
        const char* username);

    struct UserAddingResult {
        shared::user_id_t id_;
        enum class ResultType : uint8_t {
            AddedInDB,
            IncorrectUsernameFormat,
            IncorrectPasswordFormat,
        } result_;
    };
    [[nodiscard]] UserAddingResult AddUserToDB(const char* username,
                                               const char* password);

protected:
    virtual std::unique_ptr<client::ConnectionHandler> _ConnectionFactory(
        EventsHandler::ClientRawDescriptor desc);

private:
    uint32_t id_counter_ = 0;
    // TEMPORARY, TODO IMPLEMENT DATABASE
    std::vector<UserDB_Record> database_;
};
}  // namespace server
