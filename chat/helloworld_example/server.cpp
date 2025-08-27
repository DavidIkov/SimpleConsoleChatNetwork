#include <iostream>

#include "chat/events_handler.hpp"

class Client : public EventsHandler {
public:
    Client(Socket::RawDescriptorT desc) : EventsHandler(desc) {}
    void _OnEvent(EventData const& ev_data) override {
        if (ev_data.type_ == events::Type::HelloWorld) {
            auto const& data =
                *(events::StructWrapper<events::Type::HelloWorld> const*)
                     ev_data.data_;
            std::cout << "Hello world: " << data.counter_ << std::endl;
            SendEvent(events::StructWrapper<events::Type::HelloWorld>{
                data.counter_ + 1});
        }
    }
    void _OnDisconnect() override final {
        std::cout << "other side disconnected" << std::endl;
    }
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "1 parameter required: port" << std::endl;
        return -1;
    }
    uint16_t port = std::stoi(argv[1]);

    Socket::Socket_TCP server;

    auto LG = server.AquireLock();

    server.Open();
    server.BindToAddress({0, port});
    server.MarkSocketAsListening(1);

    std::cout << "opened at " << server.GetLocalAddress() << std::endl;

    Client client = server.AcceptConnection();

    {
        auto C_LG = server.AquireLock();
        std::cout << client.GetRemoteAddress() << "->"
                  << client.GetLocalAddress() << std::endl;
    }

    client.JoinReadingThread();

    return 0;
}
