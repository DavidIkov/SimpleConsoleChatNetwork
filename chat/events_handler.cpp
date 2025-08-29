
#include "events_handler.hpp"

#include <cstring>
#include <stdexcept>

EventsHandler::EventsHandler(ClientRawDescriptor desc)
    : Socket_TCP(desc.desc_),
      reading_thread_(std::thread([this]() { _ReadingThreadFunc(); })) {}
EventsHandler::~EventsHandler() {
    if (IsConnected()) Disconnect();
}
void EventsHandler::_SendData(void const* data, size_t bytes) {
    Socket_TCP::FullySendData(data, bytes);
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
            _OnEvent({*(events::Type const*)read_buffer_.data(),
                      read_buffer_.data() + sizeof(events::EnumType)});

            if (bytes)
                std::memcpy(read_buffer_.data(),
                            read_buffer_.data() + bytesToRead_ +
                                sizeof(events::EnumType),
                            bytes);

            bytes_readed_ = 0, bytes_to_read_ = 0;
        }

        if (bytes) _ProcessRawData(bytes);
    }
}

void EventsHandler::Connect(const Socket::Endpoint& endp) {
    if (IsConnected()) throw std::logic_error("socket is already connected");
    Socket_TCP::Open();
    Socket_TCP::Connect(endp);
    stopping_reading_thread_ = false;
    reading_thread_ = std::thread([this]() { _ReadingThreadFunc(); });
}

void EventsHandler::Disconnect() {
    stopping_reading_thread_ = true;
    Socket_TCP::ShutdownReading();
    Socket_TCP::Close();
    if (reading_thread_.joinable()) reading_thread_.join();
    bytes_readed_ = 0, bytes_to_read_ = 0;
}

void EventsHandler::StopThreads() {
    if (reading_thread_.joinable()) {
        stopping_reading_thread_ = true;
        Socket_TCP::ShutdownReading();
        reading_thread_.join();
    }
}

void EventsHandler::_ReadingThreadFunc() {
    while (1) {
        mutex_.lock();
        void* buff = read_buffer_.data() + bytes_readed_;
        size_t bytesToRead = read_buffer_.size() - bytes_readed_;
        size_t bytes = Socket_TCP::ReceiveData(buff, bytesToRead);
        if (!bytes) {
            if (stopping_reading_thread_) {
                return;
            } else {
                std::lock_guard LG(mutex_);
                bytes_readed_ = 0, bytes_to_read_ = 0;
                _OnDisconnect();
                Close();
                return;
            }
        } else {
            std::lock_guard LG(mutex_);
            _ProcessRawData(bytes);
        }
    }
}
