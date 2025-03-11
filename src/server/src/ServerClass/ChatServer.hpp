#pragma once
#include"EventsServer.hpp"

class ChatServerC : public EventsServerC {
private:
    using EventsServerC::IsEventsDestructorLast;
public:
    ChatServerC(asio::io_context& asioContext, asio::ip::port_type port);
    using EventsServerC::EventsServerC;
protected:
    bool IsChatDestructorLast = true;
public:
    virtual ~ChatServerC() override { if (IsChatDestructorLast) ThreadSafety.LockThread(); }
protected:
    virtual BasicClientSlotC& ClientFactory(asio::io_context& context) override;
    virtual void OnConnect(BasicClientSlotC& client) override final;
    virtual void OnDisconnect(BasicClientSlotC& client, DisconnectReasonE reason) override final;
    virtual void OnAcceptConnectionFailure() override final;
public:
    uint64_t GetUserIDByName(std::string const& username);
    std::string GetUserNameByID(uint64_t ID);
    bool CheckIfUserIDIsValid(uint64_t ID);
    enum class LogInUserResultE :unsigned char {
        InvalidID, WrongPassword, Banned, Logged
    };
    LogInUserResultE LogInUser(uint64_t userID, std::string const& password);
    uint64_t RegisterNewUser(std::string const& username, std::string const& password);
    void LogOutFromUser(uint64_t userID);
};