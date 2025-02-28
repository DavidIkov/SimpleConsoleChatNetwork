#include"EventsClient.hpp"
#include<algorithm>

EventsClientC::~EventsClientC() {
    if (IsEventsClientDestructorLast) Mutex.lock();
}
void EventsClientC::_OnReadWithOffset(size_t bytesLeft, char* start) {
    if (bytesLeft == 0) return;
    if (CurEvent.BytesReaded < sizeof(CurEvent.Type)) {
        //header of event is not fully read
        size_t headerBytesLeft = std::min(sizeof(CurEvent.Type) - CurEvent.BytesReaded, bytesLeft);
        memcpy((char*)&CurEvent.Type + CurEvent.BytesReaded, start, headerBytesLeft);
        CurEvent.BytesReaded += headerBytesLeft;
        if (CurEvent.BytesReaded == sizeof(CurEvent.Type)) {//event is identified
#define SwitchCaseTempMacro(typ) CurEvent.BytesLeftToRead=sizeof(typ);
            EventsTypesToClientSwitchCaseMacro(CurEvent.Type)
#undef SwitchCaseTempMacro
        }
        _OnReadWithOffset(bytesLeft - headerBytesLeft, start + headerBytesLeft);
    }
    else {
        size_t dataBytesLeft = std::min(CurEvent.BytesLeftToRead, bytesLeft);
        memcpy((char*)&CurEvent.Data + (CurEvent.BytesReaded - sizeof(CurEvent.Type)), start, dataBytesLeft);
        CurEvent.BytesLeftToRead -= dataBytesLeft; CurEvent.BytesReaded += dataBytesLeft;
        if (CurEvent.BytesLeftToRead == 0) {//event is finished reading
            OnEvent(CurEvent.Type, CurEvent.Data);
            CurEvent.BytesReaded = 0;
        }
        _OnReadWithOffset(bytesLeft - dataBytesLeft, start + dataBytesLeft);
    }
}