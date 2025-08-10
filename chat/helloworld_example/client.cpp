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
    Client client;
    client.Connect({127, 0, 0, 1, 55555});
    client.SendEvent(Events::StructWrapper<Events::Type::HelloWorld>{0});
    client.JoinReadingThread();
    return 0;
}
