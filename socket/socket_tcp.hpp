#pragma once
#include <stddef.h>

#include <mutex>

#include "shared.hpp"

namespace Socket {
class Socket_TCP {
public:
    Socket_TCP() = default;
    Socket_TCP(RawDescriptorT raw_descriptor) noexcept;

    Socket_TCP(Socket_TCP const &) = delete;
    Socket_TCP &operator=(Socket_TCP const &) = delete;
    Socket_TCP(Socket_TCP &&) noexcept;
    Socket_TCP &operator=(Socket_TCP &&) noexcept;
    ~Socket_TCP();

    void Open();
    void Connect(Endpoint const &endp);
    void BindToAddress(Endpoint const &endp);
    [[nodiscard]] size_t SendData(void const *data, size_t bytes);
    void FullySendData(void const *data, size_t bytes);
    // be aware that this function unlocks mutex inside, and not locking it back
    [[nodiscard]] size_t ReceiveData(void *buffer, size_t size);
    // be aware that this function unlocks mutex inside, and not locking it back
    [[nodiscard]] RawDescriptorT AcceptConnection();
    void MarkSocketAsListening(size_t queue_len);
    void Close();
    void ShutdownReading();
    void ShutdownWriting();
    [[nodiscard]] Endpoint GetLocalAddress() const;
    [[nodiscard]] Endpoint GetRemoteAddress() const;
    [[nodiscard]] inline RawDescriptorT GetDescriptor() const;

    [[nodiscard]] inline std::unique_lock<std::mutex> AquireLock() const;

protected:
    mutable std::mutex mutex_;

private:
    RawDescriptorT descriptor_ = -1;
};

RawDescriptorT Socket_TCP::GetDescriptor() const {
    std::lock_guard LG(mutex_);
    return descriptor_;
}

std::unique_lock<std::mutex> Socket_TCP::AquireLock() const {
    return std::unique_lock(mutex_);
}
}  // namespace Socket
