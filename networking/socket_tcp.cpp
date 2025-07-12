#include "socket_tcp.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdexcept>

Socket_TCP::Socket_TCP(Socket_TCP &&sock) noexcept
    : Descriptor(sock.Descriptor) {}


Socket_TCP::Socket_TCP(int socket_descriptor) noexcept
    : Descriptor(socket_descriptor) {}


Socket_TCP &Socket_TCP::operator=(Socket_TCP &&sock) noexcept {
    this->~Socket_TCP();
    return *new (this) Socket_TCP(std::move(sock));
}


Socket_TCP::~Socket_TCP() {
    if (Descriptor != -1) Close();
}


void Socket_TCP::Open() {
    if (Descriptor != -1) throw std::logic_error("socket is already opened");
    Descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (Descriptor == -1) throw std::runtime_error("failed to open socket");
}


void Socket_TCP::Connect(const char *ip, uint16_t port) {
    sockaddr_in endp_addr;
    endp_addr.sin_family = AF_INET;
    endp_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &endp_addr.sin_addr) <= 0)
        throw std::invalid_argument(
            "could not convert endpoint ip to binary form");

    if (connect(Descriptor, (sockaddr *)&endp_addr, sizeof(endp_addr)) == -1)
        throw std::runtime_error("failed to connect socket");
}


void Socket_TCP::BindToAddress(const char *ip, uint16_t port) {
    sockaddr_in localp_addr;
    localp_addr.sin_family = AF_INET;
    localp_addr.sin_port = htons(port);
    if (ip == nullptr)
        localp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    else if (inet_pton(AF_INET, ip, &localp_addr.sin_addr) <= 0)
        throw std::invalid_argument(
            "could not convert local ip to binary form");

    if (bind(Descriptor, (sockaddr *)&localp_addr, sizeof(localp_addr)) == -1)
        throw std::runtime_error("failed to change socket's local address");
}


size_t Socket_TCP::SendData(void const *data, size_t bytes) {
    ssize_t bytesSent = send(Descriptor, data, bytes, 0);
    if (bytes == -1) throw std::runtime_error("failed to send data in socket");
    return bytesSent;
}


size_t Socket_TCP::ReceiveData(void *data, size_t size) {
    if (!size) std::logic_error("cant write to empty buffer");
    ssize_t bytes = read(Descriptor, data, size);
    if (bytes == -1) throw std::runtime_error("could not read from socket");
    if (bytes == 0)
        Close();
    return bytes;
}


Socket_TCP Socket_TCP::AcceptConnection() {
    int newSocket = accept(Descriptor, nullptr, nullptr);
    if (newSocket == -1) throw std::runtime_error("could not accept socket");
    return Socket_TCP(newSocket);
}
void Socket_TCP::MarkSocketAsListening(size_t queue_len) {
    if (listen(Descriptor, queue_len) == -1) {
        throw std::runtime_error("failed to make socket start listening");
    }
}


void Socket_TCP::Close() {
    if (Descriptor == -1)
        throw std::logic_error("socket descriptor is invalid");
    if (close(Descriptor) == -1)
        throw std::runtime_error("could not close socket");
    OnDisconnect();
}
void Socket_TCP::GetLocalAddress(std::string &ip, uint16_t &port) const {
    sockaddr_in addr;
    socklen_t len = sizeof(addr);

    if (getsockname(Descriptor, (sockaddr *)&addr, &len) == -1)
        throw std::runtime_error("could not get socket local address");

    char ip_buff[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &addr.sin_addr, ip_buff, INET_ADDRSTRLEN) == nullptr)
        throw std::runtime_error(
            "could not convert socket ip from binary to text form");
    ip = ip_buff;
    port = ntohs(addr.sin_port);
}


void Socket_TCP::GetRemoteAddress(std::string &ip, uint16_t &port) const {
    sockaddr_in addr;
    socklen_t len = sizeof(addr);

    if (getpeername(Descriptor, (sockaddr *)&addr, &len) == -1)
        throw std::runtime_error("could not get socket local address");

    char ip_buff[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &addr.sin_addr, ip_buff, INET_ADDRSTRLEN) == nullptr)
        throw std::runtime_error(
            "could not convert socket ip from binary to text form");
    ip = ip_buff;
    port = ntohs(addr.sin_port);
}


