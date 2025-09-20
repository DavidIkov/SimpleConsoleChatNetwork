
#include "events/parser/parse.hpp"
#include "events/processor/processor.hpp"
#include "spdlog/spdlog.h"

class Server : public events::EventsProcessor {
public:
    Server(ClientRawDescriptor desc) : EventsProcessor(desc) {}

protected:
    virtual OutgoingRespond _ProcessRequest(IncomingRequest const &req) {
        if (req.type_id_ ==
            events::GetEventDescriptionWithName("transfer").type_id_) {
            int32_t counter = req.GetInt32(0);
            ++counter;
            SPDLOG_INFO("current counter: {}", counter);
            OutgoingRespond resp(
                "transfer",
                {events::data_types::PacketData<events::data_types::INT32>(
                    counter)});
            return resp;
        } else
            return EventsProcessor::_ProcessRequest(req);
    }
};

int main(int, char **) {
    try {
        spdlog::set_pattern("[%H:%M:%S.%e|%^%l%$|%P:%t|%s:%#] %v");
        events::ParseEventsConfig("helloworld_example/events");
        networking::socket::TCP sock;
        sock.Open();
        sock.BindToAddress({0, 0, 0, 0, 12322});
        sock.MarkSocketAsListening(1);
        Server serv(sock.AcceptConnection());
        serv.WaitTillEndOfDataProcessing();
    } catch (std::exception &err) {
        SPDLOG_ERROR("caught exception in main thread. {}", err.what());
    }
}
