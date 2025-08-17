#include "user.hpp"

#include <cstring>
#include <iostream>

namespace client {
UserHandler::~UserHandler() {
    if (!destruction_mutex_locked_) {
        destruction_mutex_locked_ = true;
        mutex_.lock();
    }
}

void UserHandler::Login(const char *username, const char *password) {
    std::lock_guard LG(mutex_);

    if (!shared::CheckUsernameSyntax(username)) {
        std::cout << "incorrect username syntax" << std::endl;
        return;
    }
    if (!shared::CheckPasswordSyntax(username)) {
        std::cout << "incorrect password syntax" << std::endl;
        return;
    }

    std::strcpy(username_, username);

    events::LoginAttempEvent login_attemp;
    std::strcpy(login_attemp.username_, username);
    std::strcpy(login_attemp.password_, password);
    _SendEvent(login_attemp);
}
void UserHandler::Logout() {
    std::lock_guard LG(mutex_);
    if (id_) {
        std::cout << "unlogged from " << id_ << " aka " << username_
                  << std::endl;
        id_ = 0;
        _SendEvent(events::LogoutEvent{});
    } else {
        std::cout << "cant unlog while not logged" << std::endl;
    }
}
void UserHandler::_OnEvent(events::Type evTyp, void const *evData) {
    if (evTyp == events::Type::LoginAttempRespond) {
        auto const &respond_data = *(events::LoginAttempRespondEvent *)evData;
        if (respond_data.id_) {
            id_ = respond_data.id_;
            switch (respond_data.response_) {
                case events::LoginAttempRespondEvent::RespondType::LoggedIn:
                    std::cout << "logged into " << id_ << " aka " << username_
                              << std::endl;
                    break;
                case events::LoginAttempRespondEvent::RespondType::
                    RegisteredAsNewUser:
                    std::cout << "logged into " << id_ << " aka " << username_
                              << " as new user" << std::endl;
                    break;
                default:
                    std::cout << "logged in, unknown respond" << std::endl;
            }

        } else {
            switch (respond_data.response_) {
                case events::LoginAttempRespondEvent::RespondType::
                    AlreadyLoggedIn:
                    std::cout << "user is already logged in" << std::endl;
                    break;
                case events::LoginAttempRespondEvent::RespondType::
                    IncorrectUsernameFormat:
                    std::cout << "incorrect username format" << std::endl;
                    break;
                case events::LoginAttempRespondEvent::RespondType::
                    IncorrectPasswordFormat:
                    std::cout << "incorrect password format" << std::endl;
                    break;
                case events::LoginAttempRespondEvent::RespondType::
                    WrongPassword:
                    std::cout << "incorrect password" << std::endl;
                    break;
                case events::LoginAttempRespondEvent::RespondType::Unknown:
                    std::cout << "unknown error" << std::endl;
                    break;
                default:
                    std::cout << "failed to login, unknown respond"
                              << std::endl;
            }
        }

    } else
        ConnectionHandler::_OnEvent(evTyp, evData);
}
}  // namespace client
