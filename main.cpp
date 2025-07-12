
#include <iostream>

#include "networking/socket_tcp.hpp"

int main(int, char** argv) {
    Socket_TCP sock;
    sock.Open();
    sock.Connect("127.0.0.1", std::stoi(argv[1]));
    char buff[100];
    size_t bytes = sock.ReceiveData(buff, 100);
    std::cout << bytes << std::endl;
    for (size_t i = 0; i < bytes; i++) std::cout << buff[i];
    std::cout << std::endl;
    std::cout << "finished" << std::endl;
    return 0;
}
