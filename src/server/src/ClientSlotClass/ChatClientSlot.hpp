#pragma once
#include"EventsClientSlot.hpp"
#include"ServerClass/ChatServer.hpp"

class ChatClientSlotC :public EventsClientSlotC {
private:
    using EventsClientSlotC::IsEventsDestructorLast;
protected:
    bool IsChatDestructorLast = true;
    ChatServerC* Server;
public:
    inline ChatClientSlotC(asio::io_context& context, ChatServerC& server):EventsClientSlotC(context), Server(&server) {
        IsEventsDestructorLast = false;
    }
    using EventsClientSlotC::EventsClientSlotC;
    virtual inline ~ChatClientSlotC() override { if (IsChatDestructorLast) ThreadSafety.LockThread(); }

protected:
    bool Logged = false;
    //user id of 0 is invalid!
    uint64_t UserID = 0;
public:
    inline bool gIsLogged() const {
        ThreadLockC TL(this); if (!TL) return false;
        return Logged;
    }

protected:
    virtual void OnConnect() override;
    virtual void OnDisconnect(DisconnectReasonE reason) override;
    virtual void OnEvent(NetworkEventsNS::EventsTypesToServerE eventType, NetworkEventsNS::EventTypeToServerU const& eventData) override;
};

