#include "user.hpp"

#include <cstring>
#include <iostream>

namespace client {

void UserHandler::Login(const char *name, const char *password) {
    if (!shared::CheckUserNameSyntax(name))
        throw std::logic_error("incorrect room name syntax");
    if (!shared::CheckUserPasswordSyntax(name))
        throw std::logic_error("incorrect password syntax");

    if (waiting_for_login_respond_)
        throw std::logic_error("already waiting for login respond");
    if (IsLoggedIn()) throw std::logic_error("already logged in");

    std::strcpy(user_.name_, name);

    events::LoginAttempEvent login_attemp;
    std::strcpy(login_attemp.name_, name);
    std::strcpy(login_attemp.password_, password);
    SendEvent(login_attemp);

    waiting_for_login_respond_ = true;

    std::unique_lock LG(mutex_, std::adopt_lock);
    event_respond_cv_.wait(
        LG, [&]() -> bool { return !waiting_for_login_respond_; });
    LG.release();
}

void UserHandler::Logout() {
    if (!IsLoggedIn()) throw std::logic_error("not logged in");

    _OnLogOut();
    user_.id_ = 0;
    SendEvent(events::LogoutEvent{});
}

void UserHandler::Disconnect() {
    if (IsLoggedIn()) Logout();
    ConnectionHandler::Disconnect();
}

void UserHandler::_OnDisconnect() {
    if (IsLoggedIn()) Logout();
    ConnectionHandler::_OnDisconnect();
}

void UserHandler::_OnEvent(EventData const &ev_data) {
    if (ev_data.type_ == events::Type::LoginAttempRespond) {
        if (!waiting_for_login_respond_) {
            std::cout << "got login attemp respond, while not waiting for it"
                      << std::endl;
            return;
        }
        auto const &respond_data =
            *(events::LoginAttempRespondEvent *)ev_data.data_;
        if (respond_data.id_) {
            user_.id_ = respond_data.id_;
            switch (respond_data.response_) {
                case events::LoginAttempRespondEvent::RespondType::LoggedIn:
                    std::cout << "logged into " << user_ << std::endl;
                    break;
                case events::LoginAttempRespondEvent::RespondType::
                    RegisteredAsNewUser:
                    std::cout << "logged into " << user_ << " as new user"
                              << std::endl;
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
                    IncorrectNameFormat:
                    std::cout << "incorrect name format" << std::endl;
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

        waiting_for_login_respond_ = false;
        event_respond_cv_.notify_all();

    } else
        ConnectionHandler::_OnEvent(ev_data);
}

void UserHandler::_OnLogOut() {
    std::cout << "unlogged from " << user_ << std::endl;
}

}  // namespace client
