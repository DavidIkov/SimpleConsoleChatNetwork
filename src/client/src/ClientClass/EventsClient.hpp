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
    using BasicClientC::Write;
    using BasicClientC::WriteResultE;
    using BasicClientC::IsBasicClientDestructorLast;
    struct{
        NetworkEventsNS::EventsTypesToClientE Type;
        NetworkEventsNS::EventTypeToClientU Data;
        //variables to not check for every possible event type when trying to get its size
        size_t BytesReaded = 0, BytesLeftToRead = 0;
    } CurEvent;
    void _OnReadWithOffset(char const* start, size_t bytesLeft);
    virtual void OnRead(char const* data, size_t len) override final {
        BasicClientC::OnRead(data, len);
        _OnReadWithOffset(data, len);
    }

#pragma region OnEvent
private:
    struct {
        void* Data = nullptr;
        void(*Callback)(void*, NetworkEventsNS::EventsTypesToClientE, NetworkEventsNS::EventTypeToClientU const&) = nullptr;
    } OnEventCallback;
public:
    void sOnEventCallback(void* data, decltype(OnEventCallback.Callback) callback){
        ThreadLockC TL(this); if(TL) OnEventCallback = { data, callback };
    }
    inline decltype(OnEventCallback) const& gOnEventCallback() const { ThreadLockC TL(this); return OnEventCallback; }
protected:
    inline virtual void OnEvent(NetworkEventsNS::EventsTypesToClientE eventType, NetworkEventsNS::EventTypeToClientU const& eventData) {
        if (OnEventCallback.Callback) OnEventCallback.Callback(OnEventCallback.Data, eventType, eventData);
    };
#pragma endregion OnEvent

public:
    using SendEventResultE = WriteResultE;
    template<NetworkEventsNS::EventsTypesToServerE EventTypeEnum>
    SendEventResultE SendEvent(const NetworkEventsNS::EventTypeToServerS<EventTypeEnum>& eventData) {
        ThreadLockC TL(this);
        if (!TL) return SendEventResultE::Canceled;
        WriteResultE res = Write(EventTypeEnum);
        if (res != WriteResultE::NoErrors) return res;
        return Write(eventData);
    }
};