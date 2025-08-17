
#include "events_handler.hpp"

#include <stdexcept>

EventsHandler::EventsHandler(ClientRawDescriptor desc)
    : Socket_TCP(desc.desc_),
      reading_thread_(std::thread([this]() { _ReadingThreadFunc(); })) {}
EventsHandler::~EventsHandler() {
    if (_GetIsConnected()) _Disconnect();
    mutex_.unlock();
}
void EventsHandler::_SendData(void const* data, size_t bytes) {
    Socket_TCP::_FullySendData(data, bytes);
}

void EventsHandler::_ProcessRawData(size_t bytes) {
    if (bytes_readed_ < sizeof(events::EnumType)) {
        size_t bytesToRead_ =
            std::min(bytes, sizeof(events::EnumType) - bytes_readed_);

        bytes_readed_ += bytesToRead_, bytes -= bytesToRead_;

        if (bytes_readed_ == sizeof(events::EnumType))
            bytes_to_read_ = events::SizesOfEvents[*(events::EnumType const*)
                                                        read_buffer_.data()];

        if (bytes) _ProcessRawData(bytes);

    } else {
        size_t bytesToRead_ = std::min(
            bytes_to_read_ + sizeof(events::EnumType) - bytes_readed_, bytes);

        bytes -= bytesToRead_, bytes_to_read_ -= bytesToRead_;

        if (!bytes_to_read_) {
            _OnEvent(*(events::Type const*)read_buffer_.data(),
                     read_buffer_.data() + sizeof(events::EnumType));
            bytes_readed_ = 0, bytes_to_read_ = 0;
        }

        if (bytes) _ProcessRawData(bytes);
    }
}

void EventsHandler::_Connect(const Socket::Endpoint& endp) {
    if (_GetIsConnected())
        throw std::logic_error("socket is already connected");
    Socket_TCP::_Open();
    Socket_TCP::_Connect(endp);
    reading_ = true;
    reading_thread_ = std::thread([this]() { _ReadingThreadFunc(); });
}

void EventsHandler::_Disconnect() {
    disconnect_in_progress_ = true;
    Socket_TCP::_ShutdownReading();
    Socket_TCP::_Close();
    reading_thread_.join();
    bytes_readed_ = 0, bytes_to_read_ = 0;
}

void EventsHandler::_ReadingThreadFunc() {
    while (1) {
        mutex_.lock();
        void* buff = read_buffer_.data() + bytes_readed_;
        size_t bytesToRead = read_buffer_.size() - bytes_readed_;
        size_t bytes = Socket_TCP::_ReceiveData(buff, bytesToRead);
        if (!bytes) {
            if (disconnect_in_progress_) {
                return;
            } else {
                mutex_.lock();
                bytes_readed_ = 0, bytes_to_read_ = 0;
                reading_ = false;
                _Close();
                _OnDisconnect();
                mutex_.unlock();
                return;
            }
        } else {
            mutex_.lock();
            _ProcessRawData(bytes);
            mutex_.unlock();
        }
    }
}

bool EventsHandler::_GetIsConnected() const {
    // when other side suddenly disconnects, thread stops execution, but is
    // still joinable, and socket is not considered reading, since reading
    // stopped.
    if (!reading_ && reading_thread_.joinable()) reading_thread_.join();
    return reading_;
}
