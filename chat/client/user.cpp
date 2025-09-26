#include "user.hpp"

#include <spdlog/spdlog.h>

using namespace events::data_types;

namespace client {

void UserHandler::LogInUser(shared::id_t id, std::string_view password) {
    if (!shared::CheckUserPasswordSyntax(password)) {
        SPDLOG_ERROR(
            "Failed to log in user. Incorrect password syntax. \"{}\".",
            password);
        throw std::exception();
    }
    if (IsLoggedIn()) {
        SPDLOG_ERROR("Failed to log in user. Already logged in.");
        throw std::exception();
    }

    IncomingRespond login_resp = SendRequest(OutgoingRequest(
        "LogInUser",
        {PacketData<INT64>(id), PacketData<STRING>(password.data())}));
    switch (login_resp.GetUInt32(0)) {
        case 0: {
            UserDB_Record user_info = GetUserDBRecordByID(id);
            if (!user_info.id_) {
                SPDLOG_ERROR(
                    "Failed to log in user. Could not get user database record "
                    "from server.");
                throw std::exception();
            }
            std::lock_guard LG(mutex_);
            user_ = std::move(user_info);
            SPDLOG_INFO("Logged into user: {}.", user_);
            break;
        }
        case 1:
            if (IsLoggedIn()) {
                SPDLOG_ERROR(
                    "Failed to log in user. Server says that user is already "
                    "logged in. Client thinks the same. Why did client sent "
                    "request?");
                throw std::exception();
            } else {
                SPDLOG_ERROR(
                    "Failed to log in user. Server says that user is already "
                    "logged in. Client thinks the opposite. Possible desync.");
                throw std::exception();
            }
        case 2:
            SPDLOG_INFO(
                "Failed to log in user. Client is logged in by other client.");
            break;
        case 3:
            SPDLOG_ERROR(
                "Failed to log in user. User thinks that password which was "
                "sent is correctly formatted, server thinks the opposite.");
            throw std::exception();
        case 4:
            SPDLOG_INFO("Failed to log in user. Incorrect password.");
            break;
        case 5:
            SPDLOG_INFO("Failed to log in user. ID does not exist.");
            break;
        default:
            SPDLOG_ERROR("Failed to log in user. Unknown respond from server.");
            throw std::exception();
    }
}

void UserHandler::LogOutOfUser() {
    if (!IsLoggedIn()) {
        SPDLOG_ERROR("Failed to log out of user. Not logged in.");
        throw std::exception();
    }

    IncomingRespond logout_resp =
        SendRequest(OutgoingRequest("LogOutOfUser", {}));

    switch (logout_resp.GetUInt32(0)) {
        case 0: {
            std::lock_guard LG(mutex_);
            SPDLOG_INFO("Unlogged from {}.", user_);
            user_.id_ = 0;
            break;
        }
        case 1:
            if (IsLoggedIn()) {
                SPDLOG_ERROR(
                    "Failed to log out of user. Server says that user is not "
                    "logged in. Client thinks the opposite. Possible desync.");
                throw std::exception();
            } else {
                SPDLOG_ERROR(
                    "Failed to log out of user. Server says that user is not "
                    "logged in. Client thinks the same. Why did client sent "
                    "request?");
                throw std::exception();
            }
        default:
            SPDLOG_ERROR(
                "Failed to log out of user. Unknown respond from server.");
            throw std::exception();
    }
}

void UserHandler::RegisterUser(std::string_view name,
                               std::string_view password) const {
    IncomingRespond register_resp = SendRequest(
        OutgoingRequest("RegisterUser", {PacketData<STRING>(name.data()),
                                         PacketData<STRING>(password.data())}));
    switch (register_resp.GetUInt32(0)) {
        case 0:
            SPDLOG_INFO(
                "Registered user with name: \"{}\" and password: \"{}\".", name,
                password);
            break;
        case 1:
            SPDLOG_ERROR(
                "Failed to register user. Client thinks that name which was "
                "sent is correctly formatted, server thinks the opposite.");
            throw std::exception();
        case 2:
            SPDLOG_ERROR(
                "Failed to register user. Client thinks that password which "
                "was "
                "sent is correctly formatted, server thinks the opposite.");
            throw std::exception();
        case 3:
            SPDLOG_INFO("Failed to register user. Name \"{}\" is already used.",
                        name);
            break;
        default:
            SPDLOG_ERROR(
                "Failed to register user. Unknown respond from server.");
            throw std::exception();
    }
}

void UserHandler::Disconnect() {
    if (IsLoggedIn()) LogOutOfUser();
    ConnectionHandler::Disconnect();
}

bool UserHandler::IsLoggedIn() const {
    std::lock_guard LG(mutex_);
    return user_.id_;
}

void UserHandler::GetUser(
    std::function<void(const UserDB_Record &)> const &callback) const {
    std::lock_guard LG(mutex_);
    callback(user_);
}

UserDB_Record UserHandler::GetUserDBRecordByID(shared::id_t id) const {
    IncomingRespond resp = SendRequest(
        OutgoingRequest("GetUserDBRecord", {PacketData<INT64>(id)}));
    if (resp.GetUInt32(0) == 0) {
        UserDB_Record rec;
        rec.id_ = id;
        rec.online_ = resp.GetBool(1);
        rec.name_ = resp.GetString(2);
        rec.create_date_ = resp.GetString(3);
        return rec;
    } else
        return {0};
}
shared::id_t UserHandler::GetUserIDByName(std::string_view name) const {
    IncomingRespond resp = SendRequest(
        OutgoingRequest("GetUserIDByName", {PacketData<STRING>(name.data())}));
    if (resp.GetUInt32(0) == 0)
        return resp.GetUInt64(1);
    else
        return 0;
}

std::string UserDB_Record::ToString() const {
    return fmt::format(
        "[ name: \"{}\", id: {}, online: {}, creation date: {} ]", name_, id_,
        online_ ? "true" : "false", create_date_);
}

}  // namespace client
