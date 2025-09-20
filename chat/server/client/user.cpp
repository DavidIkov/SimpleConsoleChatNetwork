#include "user.hpp"

#include "chat/server/users.hpp"
#include "spdlog/spdlog.h"

namespace client {
UserHandler::UserHandler(server::Base *server, ClientRawDescriptor desc)
    : ConnectionHandler(server, desc) {}

auto UserHandler::_ProcessRequest(IncomingRequest const &pack)
    -> OutgoingRespond {
    server::UsersHandler &server = *(server::UsersHandler *)server_;
    if (pack.type_id_ ==
        events::GetEventDescriptionWithName("GetUserIDByName").type_id_) {
        if (!shared::CheckUserNameSyntax(pack.GetString(0))) {
            return OutgoingRespond(
                "GetUserIDByName",
                {events::data_types::PacketData<events::data_types::UINT32>(2),
                 events::data_types::PacketData<events::data_types::INT64>(0)});
        }
        shared::id_t usr_id = server.GetUserIDByName(pack.GetString(0));
        if (!usr_id) {
            return OutgoingRespond(
                "GetUserIDByName",
                {events::data_types::PacketData<events::data_types::UINT32>(1),
                 events::data_types::PacketData<events::data_types::INT64>(0)});
        }
        return OutgoingRespond(
            "GetUserIDByName",
            {events::data_types::PacketData<events::data_types::UINT32>(0),
             events::data_types::PacketData<events::data_types::INT64>(
                 usr_id)});
    } else if (pack.type_id_ ==
               events::GetEventDescriptionWithName("GetUserDBRecord")
                   .type_id_) {
        server::UserDB_Record rec = server.GetUserByID(pack.GetInt64(0));
        if (!rec.id_) {
            return OutgoingRespond(
                "GetUserDBRecord",
                {events::data_types::PacketData<events::data_types::UINT32>(1),
                 events::data_types::PacketData<events::data_types::BOOL>(
                     false),
                 events::data_types::PacketData<events::data_types::STRING>(""),
                 events::data_types::PacketData<events::data_types::STRING>(
                     "")});
        }
        return OutgoingRespond(
            "GetUserDBRecord",
            {events::data_types::PacketData<events::data_types::UINT32>(0),
             events::data_types::PacketData<events::data_types::BOOL>(
                 rec.online_),
             events::data_types::PacketData<events::data_types::STRING>(
                 rec.name_),
             events::data_types::PacketData<events::data_types::STRING>(
                 rec.create_date_)});
    } else if (pack.type_id_ ==
               events::GetEventDescriptionWithName("RegisterUser").type_id_) {
        auto res = server.RegisterUser(pack.GetString(0), pack.GetString(1));
        switch (res.result_) {
            case decltype(res)::ResultType::NoErrors:
                return OutgoingRespond(
                    "RegisterUser",
                    {events::data_types::PacketData<events::data_types::UINT32>(
                         0),
                     events::data_types::PacketData<events::data_types::INT64>(
                         res.id_)});
            case decltype(res)::ResultType::IncorrectNameFormat:
                return OutgoingRespond(
                    "RegisterUser",
                    {events::data_types::PacketData<events::data_types::UINT32>(
                        1)});
            case decltype(res)::ResultType::IncorrectPasswordFormat:
                return OutgoingRespond(
                    "RegisterUser",
                    {events::data_types::PacketData<events::data_types::UINT32>(
                        2)});
            case decltype(res)::ResultType::NameAlreadyUsed:
                return OutgoingRespond(
                    "RegisterUser",
                    {events::data_types::PacketData<events::data_types::UINT32>(
                        3)});
            default:
                SPDLOG_WARN("Registering user resulted in unknown response.");
                return OutgoingRespond(
                    "RegisterUser",
                    {events::data_types::PacketData<events::data_types::UINT32>(
                        4)});
        }
    } else if (pack.type_id_ ==
               events::GetEventDescriptionWithName("LogInUser").type_id_) {
        std::lock_guard LG(mutex_);
        if (user_.id_) {
            return OutgoingRespond(
                "LogInUser",
                {events::data_types::PacketData<events::data_types::UINT32>(
                    1)});
        }
        if (!shared::CheckUserPasswordSyntax(pack.GetString(1))) {
            return OutgoingRespond(
                "LogInUser",
                {events::data_types::PacketData<events::data_types::UINT32>(
                    3)});
        }
        server::UserDB_Record rec = server.GetUserByID(pack.GetInt64(0));
        if (!rec.id_) {
            return OutgoingRespond(
                "LogInUser",
                {events::data_types::PacketData<events::data_types::UINT32>(
                    5)});
        }
        if (rec.online_) {
            return OutgoingRespond(
                "LogInUser",
                {events::data_types::PacketData<events::data_types::UINT32>(
                    2)});
        }
        if (rec.password_ != pack.GetString(1)) {
            return OutgoingRespond(
                "LogInUser",
                {events::data_types::PacketData<events::data_types::UINT32>(
                    4)});
        }
        server::UsersHandler::MarkUserResult mark_res =
            server.MarkUserAsOnline(rec.id_);
        switch (mark_res) {
            case server::UsersHandler::MarkUserResult::NoErrors:
                break;
            case server::UsersHandler::MarkUserResult::IDDoesNotExist:
                SPDLOG_ERROR(
                    "Failed to mark user as online. ID does not exist, but we "
                    "just checked that this id should exist.");
                throw std::exception();
            default: {
                SPDLOG_WARN(
                    "Marking user as offline resulted in unknown respond.");
                return OutgoingRespond(
                    "LogInUser",
                    {events::data_types::PacketData<events::data_types::UINT32>(
                        6)});
            }
        }
        rec.online_ = true;
        user_ = std::move(rec);

        return OutgoingRespond(
            "LogInUser",
            {events::data_types::PacketData<events::data_types::UINT32>(0)});
    } else if (pack.type_id_ ==
               events::GetEventDescriptionWithName("LogOutOfUser").type_id_) {
        std::lock_guard LG(mutex_);
        if (!user_.id_) {
            return OutgoingRespond(
                "LogOutOfUser",
                {events::data_types::PacketData<events::data_types::UINT32>(
                    1)});
        } else {
            server::UsersHandler::MarkUserResult mark_res =
                server.MarkUserAsOffline(user_.id_);
            switch (mark_res) {
                case server::UsersHandler::MarkUserResult::NoErrors:
                    break;
                case server::UsersHandler::MarkUserResult::IDDoesNotExist:
                    SPDLOG_ERROR(
                        "Failed to mark user as offline. ID does not exist, "
                        "but we just checked that this id should exist.");
                    throw std::exception();
                default: {
                    SPDLOG_WARN(
                        "Marking user as offline resulted in unknown respond.");
                    return OutgoingRespond("LogOutOfUser",
                                           {events::data_types::PacketData<
                                               events::data_types::UINT32>(2)});
                }
            }
            user_.id_ = 0;
            return OutgoingRespond(
                "LogOutOfUser",
                {events::data_types::PacketData<events::data_types::UINT32>(
                    0)});
        }
    } else
        return ConnectionHandler::_ProcessRequest(pack);
}

void UserHandler::Logout() {
    if (IsLoggedIn())
        _Logout();
    else {
        SPDLOG_ERROR("Failed to logout, client {} is not logged in.",
                     GetRemoteAddress());
        throw std::exception();
    }
}

void UserHandler::_Logout() {
    std::lock_guard LG(mutex_);
    SPDLOG_INFO("User {} logged out.", user_);
    user_.id_ = 0;
}

bool UserHandler::IsLoggedIn() const {
    std::lock_guard LG(mutex_);
    return user_.id_;
}

void UserHandler::Disconnect() {
    if (IsLoggedIn()) _Logout();
    ConnectionHandler::Disconnect();
}

}  // namespace client
