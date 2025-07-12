#pragma once

#include <array>
#include <atomic>
#include <mutex>
#include <thread>

#include "chat/events.hpp"
#include "networking/socket_tcp.hpp"

class EventsHandler : private Socket_TCP {
protected:
    EventsHandler() = default;

    std::mutex Mutex_;
    std::atomic_bool StopReadingThread_ = false;

    virtual void OnConnect() override {};
    virtual void OnDisconnect() override {};
    virtual void OnEvent(Events::Type evTyp, void const* evData) {};

    template <Events::Type EvTyp>
    void SendEvent(Events::StructWrapper<EvTyp> const& evData) {
        Socket_TCP::SendData(&evData, sizeof(evData));
    }

    void Connect(const char* ip, uint16_t port);
    void Disconnect();

private:
    void _ReadingThreadFunc();
    std::thread ReadingThread_;

    std::array<uint8_t, Events::MaxSizeOfPacket> ReadBuffer_;
    size_t BytesReaded_ = 0;
    size_t BytesToRead_ = 0;

    virtual void OnReceiveData(void* buffer, size_t bytes) override;
};
