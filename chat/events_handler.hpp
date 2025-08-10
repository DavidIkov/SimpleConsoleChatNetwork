#pragma once

#include <array>
#include <atomic>
#include <thread>

#include "chat/events.hpp"
#include "socket/socket_tcp.hpp"

class EventsHandler : private Socket::Socket_TCP {
public:
    EventsHandler() = default;
    EventsHandler(Socket::RawDescriptorT desc);
    virtual ~EventsHandler();
    EventsHandler(Socket_TCP const &) = delete;
    EventsHandler &operator=(EventsHandler const &) = delete;
    EventsHandler(EventsHandler &&) noexcept;
    EventsHandler &operator=(EventsHandler &&) noexcept;

    template <Events::Type EvTyp>
    void SendEvent(Events::StructWrapper<EvTyp> const &evData);

    inline void SendData(void const *data, size_t bytes);

    [[nodiscard]] inline bool GetIsConnected() const;

    inline void Connect(const Socket::Endpoint &endp);
    inline void Disconnect();

    inline void JoinReadingThread() const;

    using Socket_TCP::GetLocalAddress;
    using Socket_TCP::GetRemoteAddress;

protected:
    virtual void _OnEvent(Events::Type evTyp, void const *evData) = 0;

    template <Events::Type EvTyp>
    void _SendEvent(Events::StructWrapper<EvTyp> const &evData);

    void _SendData(void const *data, size_t bytes);

    [[nodiscard]] inline bool _GetIsConnected() const;

    void _Connect(const Socket::Endpoint &endp);
    void _Disconnect();

    using Socket_TCP::_GetLocalAddress;
    using Socket_TCP::_GetRemoteAddress;

    using Socket_TCP::mutex_;

private:
    void _ReadingThreadFunc();
    mutable std::thread reading_thread_;

    void _ProcessRawData(size_t bytes);

    std::atomic_bool disconnect_in_progress_ = false;

    std::array<uint8_t, Events::MaxSizeOfPacket> read_buffer_;
    size_t bytes_readed_ = 0;
    size_t bytes_to_read_ = 0;
};
template <Events::Type EvTyp>
void EventsHandler::SendEvent(Events::StructWrapper<EvTyp> const &evData) {
    std::lock_guard LG(mutex_);
    _SendEvent(evData);
}
void EventsHandler::SendData(void const *data, size_t bytes) {
    std::lock_guard LG(mutex_);
    _SendData(data, bytes);
}
bool EventsHandler::GetIsConnected() const {
    std::lock_guard LG(mutex_);
    return _GetIsConnected();
}
void EventsHandler::Connect(const Socket::Endpoint &endp) {
    std::lock_guard LG(mutex_);
    _Connect(endp);
}
void EventsHandler::Disconnect() {
    std::lock_guard LG(mutex_);
    _Disconnect();
}
void EventsHandler::JoinReadingThread() const { reading_thread_.join(); }
template <Events::Type EvTyp>
void EventsHandler::_SendEvent(Events::StructWrapper<EvTyp> const &evData) {
    EventsHandler::_SendData(&evData, sizeof(evData));
}
bool EventsHandler::_GetIsConnected() const {
    return reading_thread_.joinable();
}
