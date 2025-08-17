#include "socket_tcp.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdexcept>

using namespace Socket;

Socket_TCP::Socket_TCP(Socket_TCP &&sock) noexcept
    : descriptor_(sock.descriptor_) {}

Socket_TCP::Socket_TCP(RawDescriptorT raw_descriptor) noexcept
    : descriptor_(raw_descriptor) {}

Socket_TCP &Socket_TCP::operator=(Socket_TCP &&sock) noexcept {
    descriptor_ = sock.descriptor_;
    return *this;
}

Socket_TCP::~Socket_TCP() {
    std::lock_guard LG(mutex_);
    if (descriptor_ != -1) _Close();
}

void Socket_TCP::_Open() {
    if (descriptor_ != -1) throw std::logic_error("socket is already opened");
    descriptor_ = socket(AF_INET, SOCK_STREAM, 0);
    if (descriptor_ == -1)
        throw std::runtime_error("failed to open socket: " +
                                 std::to_string(errno));
}

void Socket_TCP::_Connect(Endpoint const &endp) {
    sockaddr_in endp_addr;
    endp_addr.sin_family = AF_INET;
    endp_addr.sin_port = htons(endp.port_);
    endp_addr.sin_addr.s_addr = htonl(endp.ip_);

    if (connect(descriptor_, (sockaddr *)&endp_addr, sizeof(endp_addr)) == -1)
        throw std::runtime_error("failed to connect socket: " +
                                 std::to_string(errno));
}

void Socket_TCP::_BindToAddress(Endpoint const &endp) {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(endp.port_);
    addr.sin_addr.s_addr = htonl(endp.ip_);

    if (bind(descriptor_, (sockaddr *)&addr, sizeof(addr)) == -1)
        throw std::runtime_error("failed to change socket's local address: " +
                                 std::to_string(errno));
}

size_t Socket_TCP::_SendData(void const *data, size_t bytes) {
    ssize_t bytesSent = send(descriptor_, data, bytes, 0);
    if (bytesSent == -1)
        throw std::runtime_error("failed to send data in socket " +
                                 std::to_string(errno));
    return bytesSent;
}

void Socket_TCP::_FullySendData(void const *data, size_t bytes) {
    while (bytes) {
        size_t bytes_sent = _SendData(data, bytes);
        data = (uint8_t *)data + bytes_sent;
        bytes -= bytes_sent;
    }
}

size_t Socket_TCP::_ReceiveData(void *data, size_t size) {
    if (!size) std::logic_error("cant write to empty buffer");
    int descriptor = descriptor_;
    mutex_.unlock();
    ssize_t bytes = read(descriptor, data, size);
    if (bytes == -1)
        throw std::runtime_error("could not read from socket: " +
                                 std::to_string(errno));
    return bytes;
}

RawDescriptorT Socket_TCP::_AcceptConnection() {
    int descriptor=descriptor_;
    mutex_.unlock();
    int newSocket = accept(descriptor, nullptr, nullptr);
    if (newSocket == -1) {
        if (errno == EBADF)
            return 0;
        else
            throw std::runtime_error("could not accept socket: " +
                                     std::to_string(errno));
    }
    return newSocket;
}
void Socket_TCP::_MarkSocketAsListening(size_t queue_len) {
    if (listen(descriptor_, queue_len) == -1) {
        throw std::runtime_error("failed to make socket start listening: " +
                                 std::to_string(errno));
    }
}

void Socket_TCP::_Close() {
    if (descriptor_ == -1)
        throw std::logic_error("socket descriptor is invalid");
    if (close(descriptor_) == -1) {
        descriptor_ = -1;
        throw std::runtime_error("could not close socket: " +
                                 std::to_string(errno));
    }
    descriptor_ = -1;
}

void Socket_TCP::_ShutdownReading() {
    if (descriptor_ == -1)
        throw std::logic_error("socket descriptor is invalid");
    if (shutdown(descriptor_, SHUT_RD) == -1)
        throw std::runtime_error("could not shutdown socket: " +
                                 std::to_string(errno));
}

void Socket_TCP::_ShutdownWriting() {
    if (descriptor_ == -1)
        throw std::logic_error("socket descriptor is invalid");
    if (shutdown(descriptor_, SHUT_WR) == -1)
        throw std::runtime_error("could not shutdown socket: " +
                                 std::to_string(errno));
}

Endpoint Socket_TCP::_GetLocalAddress() const {
    sockaddr_in addr;
    socklen_t len = sizeof(addr);

    if (getsockname(descriptor_, (sockaddr *)&addr, &len) == -1)
        throw std::runtime_error("could not get socket local address: " +
                                 std::to_string(errno));

    return {ntohl(addr.sin_addr.s_addr), ntohs(addr.sin_port)};
}

Endpoint Socket_TCP::_GetRemoteAddress() const {
    sockaddr_in addr;
    socklen_t len = sizeof(addr);

    if (getpeername(descriptor_, (sockaddr *)&addr, &len) == -1)
        throw std::runtime_error("could not get socket local address: " +
                                 std::to_string(errno));

    return {ntohl(addr.sin_addr.s_addr), ntohs(addr.sin_port)};
}
