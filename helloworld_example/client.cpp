
#include "events/parser/parse.hpp"
#include "events/processor/processor.hpp"
#include "spdlog/spdlog.h"

int main(int argc, char** argv) {
    try {
        spdlog::set_pattern("[%H:%M:%S.%e|%^%l%$|%P:%t|%s:%#] %v");
        events::ParseEventsConfig("helloworld_example/events");
        events::EventsProcessor client;
        client.Connect({0, 0, 0, 0, 12322});
        size_t counter = 0;
        while (1) {
            SPDLOG_INFO("Current count: {}", counter);
            events::EventsProcessor::OutgoingRequest req(
                "transfer",
                {events::data_types::PacketData<events::data_types::INT32>(
                    counter)});
            events::EventsProcessor::IncomingRespond resp =
                client.SendRequest(req);
            counter = resp.GetInt32(0);
            ++counter;
        }
        client.WaitTillEndOfDataProcessing();
    } catch (std::exception& err) {
        SPDLOG_ERROR("Caught exception in main thread. {}", err.what());
    }
    return 0;
}
