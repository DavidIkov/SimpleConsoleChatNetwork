#include "user.hpp"

#include <cstring>
#include <iostream>

#include "chat/server/users.hpp"

namespace client {
UserHandler::UserHandler(server::Base *server, ClientRawDescriptor desc)
    : ConnectionHandler(server, desc) {}

void UserHandler::_OnEvent(EventData const &ev_data) {
    if (ev_data.type_ == events::Type::LoginAttemp) {
        auto &loginData = *(events::LoginAttempEvent *)ev_data.data_;

        if (user_.id_) {
            SendEvent(events::LoginAttempRespondEvent{
                events::LoginAttempRespondEvent::RespondType::AlreadyLoggedIn,
                0});
            return;
        }

        server::UsersHandler &server = *(server::UsersHandler *)server_;

        auto LG = server.AquireLock();

        if (server::UsersHandler::UserDB_Record const *found_record =
                server.GetUserFromDB_FromUsername(loginData.name_)) {
            if (std::strcmp(found_record->password_, loginData.password_)) {
                std::cout << GetRemoteAddress() << " tried logging in "
                          << loginData.name_ << " with incorrect password"
                          << std::endl;

                SendEvent(events::LoginAttempRespondEvent{
                    events::LoginAttempRespondEvent::RespondType::WrongPassword,
                    0});
            } else {
                std::cout << GetRemoteAddress() << " logged in as "
                          << loginData.name_ << std::endl;

                SendEvent(events::LoginAttempRespondEvent{
                    events::LoginAttempRespondEvent::RespondType::LoggedIn,
                    found_record->id_});

                user_.id_ = found_record->id_;
                std::memcpy(user_.name_, loginData.name_,
                            std::strlen(loginData.name_));
            }
        } else {
            server::UsersHandler::UserAddingResult res =
                server.AddUserToDB(loginData.name_, loginData.password_);
            if (res.id_) {
                std::cout << GetRemoteAddress()
                          << " created a new user! say hi to him, his name is "
                          << loginData.name_ << std::endl;

                SendEvent(events::LoginAttempRespondEvent{
                    events::LoginAttempRespondEvent::RespondType::
                        RegisteredAsNewUser,
                    res.id_});

                user_.id_ = res.id_;
                std::memcpy(user_.name_, loginData.name_,
                            std::strlen(loginData.name_));

            } else {
                switch (res.result_) {
                    case server::UsersHandler::UserAddingResult::ResultType::
                        IncorrectPasswordFormat: {
                        std::cout << GetRemoteAddress()
                                  << " sent malformed password, no null "
                                     "termination. strange."
                                  << std::endl;

                        SendEvent(events::LoginAttempRespondEvent{
                            events::LoginAttempRespondEvent::RespondType::
                                IncorrectPasswordFormat,
                            0});
                        break;
                    }
                    case server::UsersHandler::UserAddingResult::ResultType::
                        IncorrectUsernameFormat: {
                        std::cout << GetRemoteAddress()
                                  << " sent malformed username, no null "
                                     "termination. strange."
                                  << std::endl;

                        SendEvent(events::LoginAttempRespondEvent{
                            events::LoginAttempRespondEvent::RespondType::
                                IncorrectNameFormat,
                            0});
                        break;
                    }
                    default: {
                        std::cout << GetRemoteAddress()
                                  << "'s attemp to login as " << loginData.name_
                                  << " resulted in unknown respond"
                                  << std::endl;

                        SendEvent(events::LoginAttempRespondEvent{
                            events::LoginAttempRespondEvent::RespondType::
                                Unknown,
                            0});
                    }
                }
            }
        }
    } else if (ev_data.type_ == events::Type::Logout) {
        if (IsLoggedIn()) {
            _OnLogOut();
            user_.id_ = 0;
        } else
            std::cout << GetRemoteAddress()
                      << " tried to logout while not logged in" << std::endl;
    } else
        ConnectionHandler::_OnEvent(ev_data);
}

void UserHandler::_OnLogOut() {
    std::cout << user_ << " logged out" << std::endl;
}

}  // namespace client
