#include"ChatClientSlot.hpp"

class ChatClientSlotC :public EventsClientSlotC {
private:
    using EventsClientSlotC::IsEventsDestructorLast;
protected:
    bool IsChatDestructorLast = true;
public:
    inline ChatClientSlotC(asio::io_context& context):EventsClientSlotC(context){
        IsEventsDestructorLast = false;
    }
    using EventsClientSlotC::EventsClientSlotC;
    virtual inline ~ChatClientSlotC() override { if (IsChatDestructorLast) ThreadSafety.LockThread(); }

public:

};

