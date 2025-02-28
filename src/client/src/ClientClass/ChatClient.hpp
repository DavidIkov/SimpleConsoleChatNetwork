#pragma once
#include"EventsClient.hpp"

class ChatClientC :public EventsClientC {
private:
    using EventsClientC::IsEventsClientDestructorLast;
    using EventsClientC::OnEvent;
    using EventsClientC::SendEvent;
public:
    ChatClientC(asio::io_context& context);
    using EventsClientC::EventsClientC;
protected:
    bool IsChatClientDestructorLast = true;
public:
    virtual ~ChatClientC() override;
private:
    virtual void OnEvent(NetworkEventsNS::EventsTypesToClientE eventType, NetworkEventsNS::EventTypeToClientU const& eventData) override final;
private:
    struct {
        bool Happening = false, ServerResponded = false;
    } LoggingInUserInServerEvent;
    bool LoggedInUserInServer = false;
protected:
    inline bool _gIsLoggedInUserInServer() const noexcept { return LoggedInUserInServer; }
public:
    inline bool gIsLoggedInUserInServer() const noexcept { std::lock_guard lg(Mutex); return _gIsLoggedInUserInServer(); }
    enum class LogInResultE :unsigned char {
        NoErrors, NotConnected, FailedSendingEvent, AlreadyLogged, UsernameTooLong, PasswordTooLong
    };
    inline LogInResultE LogIn(char const* username, char const* password) { return LogIn(std::string(username), std::string(password)); }
    LogInResultE LogIn(std::string username, std::string password);
    enum class LogOutResultE :unsigned char {
        NoErrors, NotConnected, NotLoggedIn
    };
    LogOutResultE LogOut();


private:
};