#pragma once
#include <stddef.h>

#include <cstdint>
#include <string>

//todo add shutdown for gracefull disconnect
class Socket_TCP {
private:
    int Descriptor = -1;

public:
    Socket_TCP() = default;

private:
    Socket_TCP(int socket_descriptor) noexcept;

public:
    Socket_TCP(Socket_TCP const &) = delete;
    Socket_TCP &operator=(Socket_TCP const &) = delete;
    Socket_TCP(Socket_TCP &&) noexcept;
    Socket_TCP &operator=(Socket_TCP &&) noexcept;
    virtual ~Socket_TCP();

    void Open();
    void Connect(const char *ip, uint16_t port);
    void BindToAddress(const char *ip, uint16_t port);
    size_t SendData(void const *data, size_t bytes);
    size_t ReceiveData(void *buffer, size_t size);
    Socket_TCP AcceptConnection();
    void MarkSocketAsListening(size_t queue_len);
    void Close();
    void GetLocalAddress(std::string &ip, uint16_t &port) const;
    void GetRemoteAddress(std::string &ip, uint16_t &port) const;

protected:
    virtual void OnDisconnect() {};
};
