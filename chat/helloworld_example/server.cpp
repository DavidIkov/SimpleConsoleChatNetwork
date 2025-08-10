#include <iostream>

#include "chat/events_handler.hpp"

class Client : public EventsHandler {
public:
    Client(Socket::RawDescriptorT desc) : EventsHandler(desc) {}
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
    Socket::Socket_TCP server;
    server.Open();
    server.BindToAddress({0, 55555});
    server.MarkSocketAsListening(1);
    Client client = server.AcceptConnection();
    std::cout << client.GetRemoteAddress() << "->" << client.GetLocalAddress()
              << std::endl;

    client.JoinReadingThread();

    return 0;
}
