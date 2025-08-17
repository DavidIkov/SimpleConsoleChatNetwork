#include "user.hpp"

#include <cstring>
#include <iostream>

#include "chat/server/users.hpp"

namespace client {
UserHandler::UserHandler(server::ConnectionsHandler *server,
                         ClientRawDescriptor desc)
    : ConnectionHandler(server, desc) {}

UserHandler::~UserHandler() {
    if (!destruction_mutex_locked_) {
        destruction_mutex_locked_ = true;
        mutex_.lock();
    }
}

void UserHandler::_OnEvent(events::Type evTyp, void const *evData) {
    if (evTyp == events::Type::LoginAttemp) {
        if (id_) {
            _SendEvent(events::LoginAttempRespondEvent{
                events::LoginAttempRespondEvent::RespondType::AlreadyLoggedIn,
                0});
            return;
        }

        auto &loginData = *(events::LoginAttempEvent *)evData;

        server::UsersHandler &server = *(server::UsersHandler *)server_;

        if (server::UsersHandler::UserDB_Record const *found_record =
                server.GetUserFromDB_FromUsername(loginData.username_)) {
            if (std::strcmp(found_record->password_, loginData.password_)) {
                std::cout << _GetRemoteAddress() << " tried logging in "
                          << loginData.username_ << " with incorrect password"
                          << std::endl;

                _SendEvent(events::LoginAttempRespondEvent{
                    events::LoginAttempRespondEvent::RespondType::WrongPassword,
                    0});
            } else {
                std::cout << _GetRemoteAddress() << " logged in as "
                          << loginData.username_ << std::endl;

                _SendEvent(events::LoginAttempRespondEvent{
                    events::LoginAttempRespondEvent::RespondType::LoggedIn,
                    found_record->id_});
            }
        } else {
            server::UsersHandler::UserAddingResult res =
                server.AddUserToDB(loginData.username_, loginData.password_);
            if (res.id_) {
                std::cout << _GetRemoteAddress()
                          << " created a new user! say hi to him, his name is "
                          << loginData.username_ << std::endl;

                _SendEvent(events::LoginAttempRespondEvent{
                    events::LoginAttempRespondEvent::RespondType::
                        RegisteredAsNewUser,
                    res.id_});
            } else {
                switch (res.result_) {
                    case server::UsersHandler::UserAddingResult::ResultType::
                        IncorrectPasswordFormat: {
                        std::cout << _GetRemoteAddress()
                                  << " sent malformed password, no null "
                                     "termination. strange."
                                  << std::endl;

                        _SendEvent(events::LoginAttempRespondEvent{
                            events::LoginAttempRespondEvent::RespondType::
                                IncorrectPasswordFormat,
                            0});
                        break;
                    }
                    case server::UsersHandler::UserAddingResult::ResultType::
                        IncorrectUsernameFormat: {
                        std::cout << _GetRemoteAddress()
                                  << " sent malformed username, no null "
                                     "termination. strange."
                                  << std::endl;

                        _SendEvent(events::LoginAttempRespondEvent{
                            events::LoginAttempRespondEvent::RespondType::
                                IncorrectUsernameFormat,
                            0});
                        break;
                    }
                    default: {
                        std::cout
                            << _GetRemoteAddress() << "'s attemp to login as "
                            << loginData.username_
                            << " resulted in unknown respond" << std::endl;

                        _SendEvent(events::LoginAttempRespondEvent{
                            events::LoginAttempRespondEvent::RespondType::
                                Unknown,
                            0});
                    }
                }
            }
        }
    } else if (evTyp == events::Type::Logout) {
        if (!id_)
            std::cout << _GetRemoteAddress()
                      << " tried to logout while no logged in" << std::endl;
        else {
            std::cout << "user " << id_ << " aka " << username_ << " aka "
                      << _GetRemoteAddress() << " logged out" << std::endl;
            id_ = 0;
        }
    } else
        ConnectionHandler::_OnEvent(evTyp, evData);
}
}  // namespace client
