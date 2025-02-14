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
    
private:
    bool WaitingForLogin = false;
    bool RegisteredInServer = false;//means that login was correct and so server accepted it
public:
    inline bool gIsRegisteredInServer() const noexcept { return RegisteredInServer; }
    inline bool gIsWaitingForLogin() const noexcept { return WaitingForLogin; }

private:
};