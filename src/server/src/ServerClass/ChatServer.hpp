#pragma once
#include"EventsServer.hpp"

class ChatServerC :virtual public EventsServerC {
public:
    using EventsServerC::EventsServerC;
    virtual ~ChatServerC() override = default;
protected:
    struct ChatClientS :virtual public BasicClientS {
        std::mutex EventMutex;
        bool WaitingForLogin = false;
        bool Registered = false;//means that login was correct and so server accepted it
        using BasicClientS::BasicClientS;
        virtual ~ChatClientS() { std::lock_guard lg(EventMutex); }
    };
    virtual BasicClientS& ClientFactory() override;
private:
    virtual void OnEvent(BasicClientS& client, NetworkEventsNS::EventsTypesToServerE eventType, NetworkEventsNS::EventTypeToServerU const& eventData) override final;
};