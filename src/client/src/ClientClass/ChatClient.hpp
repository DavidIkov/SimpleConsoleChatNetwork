#pragma once
#include"EventsClient.hpp"

class ChatClientC :virtual public EventsClientC {
public:
    using EventsClientC::EventsClientC;
    virtual ~ChatClientC() override = default;
private:
    virtual void OnEvent(NetworkEventsNS::EventsTypesToClientE eventType, NetworkEventsNS::EventTypeToClientU const& eventData) override final;
private:
    virtual void OnConnect() override final;
    virtual void OnDisconnect() override final;
    std::recursive_mutex EventMutex;

private:
    bool LoggedInUserInServer = false;
public:
    inline bool gIsLoggedInUserInServer() const noexcept { return LoggedInUserInServer; }
    enum class LogInResultE :unsigned char {
        NoErrors, NotConnectedToServer, AlreadyLogged, UsernameTooLong, PasswordTooLong
    };
    inline LogInResultE LogIn(char const* username, char const* password) { return LogIn(std::string(username), std::string(password)); }
    LogInResultE LogIn(std::string username, std::string password);
    enum class LogOutResultE :unsigned char {
        NoErrors, NotConnectedToServer, NotLoggedIn
    };
    LogOutResultE LogOut();


private:
};