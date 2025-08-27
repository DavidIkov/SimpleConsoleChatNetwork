#pragma once

#include <array>
#include <atomic>
#include <thread>

#include "chat/events.hpp"
#include "socket/socket_tcp.hpp"

class EventsHandler : private Socket::Socket_TCP {
public:
    /*
     When server accepts connection, it may return raw descriptor, to use this
     descriptor, use this struct. This is the only scenario where this struct
     should be used. Dont use this with just raw descriptors that were aquired
     in any other scenario.
     */
    struct ClientRawDescriptor {
        Socket::RawDescriptorT desc_;
        ClientRawDescriptor(Socket::RawDescriptorT desc) : desc_(desc) {}
    };

    EventsHandler() = default;
    EventsHandler(ClientRawDescriptor desc);
    virtual ~EventsHandler();
    EventsHandler(EventsHandler const &) = delete;
    EventsHandler &operator=(EventsHandler const &) = delete;
    EventsHandler(EventsHandler &&) noexcept;
    EventsHandler &operator=(EventsHandler &&) noexcept;

    template <events::Type EvTyp>
    inline void SendEvent(events::StructWrapper<EvTyp> const &evData);

    [[nodiscard]] bool GetIsConnected() const;

    void Connect(const Socket::Endpoint &endp);
    virtual void Disconnect();

    inline void JoinReadingThread() const;

    using Socket_TCP::AquireLock;
    using Socket_TCP::GetLocalAddress;
    using Socket_TCP::GetRemoteAddress;

    struct EventData {
        events::Type type_;
        void const *data_;
    };

protected:
    virtual void _OnEvent(EventData const &ev_data) = 0;

    // happens on unpredicted disconnect, for example other side closing
    // connection.
    virtual void _OnDisconnect() = 0;

    using Socket_TCP::mutex_;

private:
    void _ReadingThreadFunc();
    mutable std::thread reading_thread_;

    void _SendData(void const *data, size_t bytes);

    bool reading_ = false;

    void _ProcessRawData(size_t bytes);

    std::atomic_bool disconnect_in_progress_ = false;

    std::array<uint8_t, events::MaxSizeOfPacket> read_buffer_;
    size_t bytes_readed_ = 0;
    size_t bytes_to_read_ = 0;
};

void EventsHandler::JoinReadingThread() const { reading_thread_.join(); }

template <events::Type EvTyp>
void EventsHandler::SendEvent(events::StructWrapper<EvTyp> const &evData) {
    auto EvTypLV = EvTyp;
    EventsHandler::_SendData(&EvTypLV, sizeof(EvTypLV));
    EventsHandler::_SendData(&evData, sizeof(evData));
}
