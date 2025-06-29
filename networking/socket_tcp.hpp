#pragma once
#include <stddef.h>

#include <cstdint>
#include <string>

class CSocket_TCP
{
private:
    int Descriptor = -1;

public:
    CSocket_TCP() = default;

private:
    CSocket_TCP(int socket_descriptor) noexcept;

public:
    CSocket_TCP(CSocket_TCP const &) = delete;
    CSocket_TCP(CSocket_TCP &&) noexcept;
    CSocket_TCP &operator=(CSocket_TCP const &) = delete;
    CSocket_TCP &operator=(CSocket_TCP &&) noexcept;
    ~CSocket_TCP();

    void Open();
    void Connect(const char *ip, uint16_t port);
    void BindToAddress(const char *ip, uint16_t port);
    size_t SendData(void const *data, size_t bytes);
    size_t ReceiveData(void *buffer, size_t size);
    CSocket_TCP AcceptConnection();
    void MarkSocketAsListening(size_t queue_len);
    void Close();
    void GetLocalAddress(std::string& ip, uint16_t& port) const;
    void GetRemoteAddress(std::string& ip, uint16_t& port) const;
};
