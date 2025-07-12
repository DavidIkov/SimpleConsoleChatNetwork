
#include <iostream>

#include "networking/socket_tcp.hpp"

int main(int, char**) {
    Socket_TCP sock;
    sock.Open();
    sock.BindToAddress("127.0.0.1", 0);
    {
        std::string ip;
        uint16_t port;
        sock.GetLocalAddress(ip, port);
        std::cout << "port: " << port << std::endl;
    }
    sock.MarkSocketAsListening(10);
    Socket_TCP client = sock.AcceptConnection();
    std::cout << "acceptet" << std::endl;
    client.SendData("huh", 4);
    std::cout << "finished" << std::endl;
    return 0;
}
