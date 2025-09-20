#pragma once
#include <stddef.h>

#include <mutex>

#include "common.hpp"

namespace networking::socket {
class TCP {
public:
    TCP() = default;
    // used to work with sockets that are accepted. used on server side.
    TCP(RawDescriptorT raw_descriptor) noexcept;

    TCP(TCP const &) = delete;
    TCP &operator=(TCP const &) = delete;
    TCP(TCP &&) noexcept;
    TCP &operator=(TCP &&) noexcept;
    ~TCP();

    void Open();
    void Connect(Endpoint const &endp);
    void BindToAddress(Endpoint const &endp);
    void SendData(void const *data, size_t bytes) const;
    [[nodiscard]] size_t ReceiveData(void *buffer, size_t size) const;
    [[nodiscard]] RawDescriptorT AcceptConnection()const;
    void MarkSocketAsListening(size_t queue_len);
    void Close();
    void ShutdownReading();
    void ShutdownWriting();
    [[nodiscard]] Endpoint GetLocalAddress() const;
    [[nodiscard]] Endpoint GetRemoteAddress() const;
    [[nodiscard]] RawDescriptorT GetDescriptor() const;
    [[nodiscard]] bool IsConnected() const;
    [[nodiscard]] bool IsOpened() const;

private:
    mutable std::mutex mutex_;

    bool connected_ = false;

    RawDescriptorT descriptor_ = -1;
};

}  // namespace networking::socket
