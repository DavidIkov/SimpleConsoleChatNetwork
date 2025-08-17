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

    inline void Open();
    inline void Connect(Endpoint const &endp);
    inline void BindToAddress(Endpoint const &endp);
    [[nodiscard]] inline size_t SendData(void const *data, size_t bytes);
    inline void FullySendData(void const *data, size_t bytes);
    [[nodiscard]] inline size_t ReceiveData(void *buffer, size_t size);
    [[nodiscard]] inline RawDescriptorT AcceptConnection();
    inline void MarkSocketAsListening(size_t queue_len);
    inline void Close();
    inline void ShutdownReading();
    inline void ShutdownWriting();
    [[nodiscard]] inline Endpoint GetLocalAddress() const;
    [[nodiscard]] inline Endpoint GetRemoteAddress() const;
    [[nodiscard]] inline RawDescriptorT GetDescriptor() const;

protected:
    mutable std::mutex mutex_;

    void _Open();
    void _Connect(Endpoint const &endp);
    void _BindToAddress(Endpoint const &endp);
    [[nodiscard]] size_t _SendData(void const *data, size_t bytes);
    void _FullySendData(void const *data, size_t bytes);
    // be aware that this function unlocks mutex inside, and not locking it back
    [[nodiscard]] size_t _ReceiveData(void *buffer, size_t size);
    // be aware that this function unlocks mutex inside, and not locking it back
    [[nodiscard]] RawDescriptorT _AcceptConnection();
    void _MarkSocketAsListening(size_t queue_len);
    void _Close();
    void _ShutdownReading();
    void _ShutdownWriting();
    [[nodiscard]] Endpoint _GetLocalAddress() const;
    [[nodiscard]] Endpoint _GetRemoteAddress() const;

private:
    RawDescriptorT descriptor_ = -1;
};

void Socket_TCP::Open() {
    std::lock_guard LG(mutex_);
    _Open();
}
void Socket_TCP::Connect(Endpoint const &endp) {
    std::lock_guard LG(mutex_);
    _Connect(endp);
}
void Socket_TCP::BindToAddress(Endpoint const &endp) {
    std::lock_guard LG(mutex_);
    _BindToAddress(endp);
}
size_t Socket_TCP::SendData(void const *data, size_t bytes) {
    std::lock_guard LG(mutex_);
    return _SendData(data, bytes);
}
void Socket_TCP::FullySendData(void const *data, size_t bytes) {
    std::lock_guard LG(mutex_);
    _FullySendData(data, bytes);
}
size_t Socket_TCP::ReceiveData(void *buffer, size_t size) {
    mutex_.lock();
    return _ReceiveData(buffer, size);
}
RawDescriptorT Socket_TCP::AcceptConnection() {
    mutex_.lock();
    return _AcceptConnection();
}
void Socket_TCP::MarkSocketAsListening(size_t queue_len) {
    std::lock_guard LG(mutex_);
    _MarkSocketAsListening(queue_len);
}
void Socket_TCP::Close() {
    std::lock_guard LG(mutex_);
    _Close();
}
void Socket_TCP::ShutdownReading() {
    std::lock_guard LG(mutex_);
    _ShutdownReading();
}
void Socket_TCP::ShutdownWriting() {
    std::lock_guard LG(mutex_);
    _ShutdownWriting();
}
Endpoint Socket_TCP::GetLocalAddress() const {
    std::lock_guard LG(mutex_);
    return _GetLocalAddress();
}
Endpoint Socket_TCP::GetRemoteAddress() const {
    std::lock_guard LG(mutex_);
    return _GetRemoteAddress();
}
RawDescriptorT Socket_TCP::GetDescriptor() const {
    std::lock_guard LG(mutex_);
    return descriptor_;
}
}  // namespace Socket
