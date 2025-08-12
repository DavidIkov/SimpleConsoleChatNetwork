#include <iostream>

#include "chat/events_handler.hpp"

class Client : public EventsHandler {
public:
    void _OnEvent(Events::Type evTyp, void const* evData) override {
        if (evTyp == Events::Type::HelloWorld) {
            auto const& data =
                *(Events::StructWrapper<Events::Type::HelloWorld> const*)evData;
            std::cout << "Hello world: " << data.num << std::endl;
            _SendEvent(
                Events::StructWrapper<Events::Type::HelloWorld>{data.num + 1});
        }
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
    client.Connect({octet1, octet2, octet3, octet4, port});

    std::cout << "Connected! " << client.GetLocalAddress() << "->"
              << client.GetRemoteAddress() << std::endl;

    client.SendEvent(Events::StructWrapper<Events::Type::HelloWorld>{0});
    client.JoinReadingThread();
    return 0;
}
