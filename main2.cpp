
#include <iostream>

#include "socket/socket_tcp.hpp"

int main(int, char**) {
    Socket::Socket_TCP sock;
    sock.Open();
    sock.BindToAddress({127, 0, 0, 1, 0});
    {
        Socket::Endpoint endp = sock.GetLocalAddress();
        std::cout << "port: " << endp.port_ << std::endl;
    }
    sock.MarkSocketAsListening(10);
    Socket::Socket_TCP client = sock.AcceptConnection();
    std::cout << "acceptet" << std::endl;
    client.FullySendData("huh", 4);
    std::cout << "finished" << std::endl;
    return 0;
}
