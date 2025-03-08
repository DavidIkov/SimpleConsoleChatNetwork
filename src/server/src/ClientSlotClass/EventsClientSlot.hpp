#pragma once
#include"BasicClientSlot.hpp"
#include"NetworkEvents.hpp"

class EventsClientSlotC : public BasicClientSlotC {
private:
    using BasicClientSlotC::IsBasicDestructorLast;
    using BasicClientSlotC::Write;
    using BasicClientSlotC::WriteResultE;
protected:
    bool IsEventsDestructorLast = true;
public:
    inline EventsClientSlotC(asio::io_context& context):BasicClientSlotC(context){
        IsBasicDestructorLast = false;
    }
    using BasicClientSlotC::BasicClientSlotC;
    virtual inline ~EventsClientSlotC() override { if (IsEventsDestructorLast) ThreadSafety.LockThread(); }
private:
    struct{
        NetworkEventsNS::EventsTypesToServerE Type;
        NetworkEventsNS::EventTypeToServerU Data;
        //variables to not check for every possible event type when trying to get its size
        size_t BytesReaded = 0, BytesLeftToRead = 0;
    } CurEvent;
    void _OnReadWithOffset(char const* start, size_t bytesLeft);
    virtual void OnRead(char const* data, size_t len) override final {
        BasicClientSlotC::OnRead(data, len);
        _OnReadWithOffset(data, len);
    }

#pragma region OnEvent
private:
    struct {
        void* Data = nullptr;
        void(*Callback)(EventsClientSlotC*, void*, NetworkEventsNS::EventsTypesToServerE, NetworkEventsNS::EventTypeToServerU const&) = nullptr;
    } OnEventCallback;
public:
    void sOnEventCallback(void* data, decltype(OnEventCallback.Callback) callback){
        ThreadLockC TL(this); if(TL) OnEventCallback = { data, callback };
    }
    inline decltype(OnEventCallback) const& gOnEventCallback() const { ThreadLockC TL(this); return OnEventCallback; }
protected:
    inline virtual void OnEvent(NetworkEventsNS::EventsTypesToServerE eventType, NetworkEventsNS::EventTypeToServerU const& eventData) {
        if (OnEventCallback.Callback) OnEventCallback.Callback(this, OnEventCallback.Data, eventType, eventData);
    };
#pragma endregion OnEvent

public:
    using SendEventResultE = WriteResultE;
    template<NetworkEventsNS::EventsTypesToClientE EventTypeEnum>
    SendEventResultE SendEvent(const NetworkEventsNS::EventTypeToClientS<EventTypeEnum>& eventData) {
        ThreadLockC TL(this);
        if (!TL) return SendEventResultE::Canceled;
        WriteResultE res = Write(EventTypeEnum);
        if (res != WriteResultE::NoErrors) return res;
        return Write(eventData);
    }
};