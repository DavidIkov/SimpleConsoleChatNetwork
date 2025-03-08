#include"EventsServer.hpp"
#include"ClientSlotClass/EventsClientSlot.hpp"

void EventsServerC::SetUpCallbacksForNewClient(BasicClientSlotC& client) {
    BasicServerC::SetUpCallbacksForNewClient(client);
    dynamic_cast<EventsClientSlotC&>(client).sOnEventCallback(this,
        [](EventsClientSlotC* client, void* ptr, NetworkEventsNS::EventsTypesToServerE type, NetworkEventsNS::EventTypeToServerU const& data) {
            EventsServerC* serv = (EventsServerC*)ptr;
            ThreadLockC TL(serv);
            serv->OnEvent(*client, type, data);
        });
}
BasicClientSlotC& EventsServerC::ClientFactory(asio::io_context& context) {
    return *Clients.emplace_back(new EventsClientSlotC(context)).get();
}