#include <iostream>

#include "chat/events_handler.hpp"

class Client : public EventsHandler {
public:
    void _OnEvent(EventData const& ev_data) override {
        if (ev_data.type_ == events::Type::HelloWorld) {
            auto const& data =
                *(events::StructWrapper<events::Type::HelloWorld> const*)ev_data.data_;
            std::cout << "Hello world: " << data.counter_ << std::endl;
            SendEvent(
                events::StructWrapper<events::Type::HelloWorld>{data.counter_ + 1});
        }
    }
    void _OnDisconnect() override final {
        std::cout << "other side disconnected" << std::endl;
    }
};

int main(int argc, char** argv) {
    if (argc != 6) {
        std::cout << "5 parameters required: octet 1-4, port" << std::endl;
        return -1;
    }
    uint8_t octet1 = std::stoi(argv[1]), octet2 = std::stoi(argv[2]),
            octet3 = std::stoi(argv[3]), octet4 = std::stoi(argv[4]);
    uint16_t port = std::stoi(argv[5]);

    Client client;

    auto LG = client.AquireLock();

    {
        client.Connect({octet1, octet2, octet3, octet4, port});

        std::cout << "Connected! " << client.GetLocalAddress() << "->"
                  << client.GetRemoteAddress() << std::endl;

        client.SendEvent(events::StructWrapper<events::Type::HelloWorld>{0});
    }
    client.JoinReadingThread();
    return 0;
}
