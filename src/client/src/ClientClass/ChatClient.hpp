#pragma once
#include"EventsClient.hpp"

class ChatClientC :public EventsClientC {
private:
    using EventsClientC::IsEventsClientDestructorLast;
    using EventsClientC::OnEvent;
    using EventsClientC::SendEvent;
    virtual void OnConnect() override;
    virtual void OnDisconnect(DisconnectReasonE) override;
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
        bool Active = false, ServerResponded = false, Stopped = false;
        NetworkEventsNS::EventTypeToClientS<NetworkEventsNS::EventsTypesToClientE::LogInResult>::RespTypeE ResponseType;
    } LoggingInUserEvent;
    bool LoggedInUser = false;
protected:
    inline bool _gIsLoggedInUser() const noexcept { return LoggedInUser; }
public:
    inline bool gIsLoggedInUser() const noexcept { ThreadLockC TL(this); return TL && _gIsLoggedInUser(); }
    enum class LogInResultE :unsigned char {
        NotConnected, FailedSendingEvent, Canceled, AlreadyLogged, UsernameTooLong, PasswordTooLong,
        Banned, WrongPassword, LoggedAsNewUser, LoggedAsExistingUser, UnknownRespond
    };
    inline LogInResultE LogIn(char const* username, char const* password) { return LogIn(std::string(username), std::string(password)); }
    LogInResultE LogIn(std::string username, std::string password);
    enum class LogOutResultE :unsigned char {
        Canceled, NoErrors, NotConnected, NotLoggedIn, UnknownError
    };
    LogOutResultE LogOut();
};