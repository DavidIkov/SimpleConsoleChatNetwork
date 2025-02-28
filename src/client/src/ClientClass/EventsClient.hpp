#pragma once
#include"BasicClient.hpp"
#include"NetworkEvents.hpp"
//client that is working with events from NetworkEvents.hpp
class EventsClientC :public BasicClientC {
public:
    inline EventsClientC(asio::io_context& context):BasicClientC(context){
        IsBasicClientDestructorLast = false;
    }
    using BasicClientC::BasicClientC;
protected:
    bool IsEventsClientDestructorLast = true;
public:
    virtual ~EventsClientC() override;
private:
    using BasicClientC::SocketBuffer;
    using BasicClientC::Write;
    using BasicClientC::WriteResultE;
    using BasicClientC::OnConnect;
    using BasicClientC::OnDisconnect;
    using BasicClientC::IsBasicClientDestructorLast;
    inline virtual void OnConnect() override final {
        BasicClientC::OnConnect();
        OnEvent(NetworkEventsNS::EventsTypesToClientE::ConnectedToServer, {});
    }
    inline virtual void OnDisconnect(DisconnectReasonE reason) override final {
        BasicClientC::OnDisconnect(reason);
        NetworkEventsNS::EventTypeToClientU ev; ev.DisconnectedFromServer = { reason };
        OnEvent(NetworkEventsNS::EventsTypesToClientE::DisconnectedFromServer, ev);
    };
    struct{
        NetworkEventsNS::EventsTypesToClientE Type;
        NetworkEventsNS::EventTypeToClientU Data;
        //variables to not check for every possible event type when trying to get its size
        size_t BytesReaded = 0, BytesLeftToRead = 0;
    } CurEvent;
    void _OnReadWithOffset(size_t bytesLeft, char* start);
    virtual void OnRead(size_t bytesRead) override final { _OnReadWithOffset(bytesRead, SocketBuffer); }
protected:
    inline virtual void OnEvent(NetworkEventsNS::EventsTypesToClientE eventType, NetworkEventsNS::EventTypeToClientU const& eventData) {};
public:
    using SendEventResultE = WriteResultE;
protected:
    template<NetworkEventsNS::EventsTypesToServerE EventTypeEnum>
    SendEventResultE _SendEvent(const NetworkEventsNS::EventTypeToServerS<EventTypeEnum>& eventData) {
        WriteResultE res = _Write(EventTypeEnum);
        if (res != WriteResultE::NoErrors) return res;
        return _Write(eventData);
    }
public:
    template<NetworkEventsNS::EventsTypesToServerE EventTypeEnum>
    SendEventResultE SendEvent(const NetworkEventsNS::EventTypeToServerS<EventTypeEnum>& eventData) {
        std::lock_guard lg(Mutex);
        return _SendEvent(eventData);
    }
};