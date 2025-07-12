
#include "socket.hpp"

void EventsHandler::OnReceiveData(void*, size_t bytes) {
    if (BytesReaded_ < sizeof(Events::EnumType)) {
        size_t bytesToRead_ =
            std::min(bytes, sizeof(Events::EnumType) - BytesReaded_);

        BytesReaded_ += bytesToRead_, bytes -= bytesToRead_;

        if (BytesReaded_ == sizeof(Events::EnumType))
            BytesToRead_ =
                Events::SizesOfEvents[*(Events::Type const*)ReadBuffer_.data()];

        if (bytes) OnReceiveData(nullptr, bytes);

    } else {
        size_t bytesToRead_ = std::min(
            BytesToRead_ + sizeof(Events::EnumType) - BytesReaded_, bytes);

        bytes -= bytesToRead_, BytesToRead_ -= bytesToRead_;

        if (!BytesToRead_) {
            OnEvent(*(Events::Type const*)ReadBuffer_.data(),
                    ReadBuffer_.data() + sizeof(Events::EnumType));
            BytesReaded_ = 0, BytesToRead_ = 0;
        }

        if (bytes) OnReceiveData(nullptr, bytes);
    }
}

void EventsHandler::Connect(const char* ip, uint16_t port) {
    StopReadingThread_ = false;
    ReadingThread_ = std::thread([this]() { _ReadingThreadFunc(); });
    Socket_TCP::Connect(ip, port);
}
void EventsHandler::Disconnect() {
    StopReadingThread_ = true;
    Socket_TCP::Close();
    ReadingThread_.join();
}
void EventsHandler::_ReadingThreadFunc(){
    std::unique_lock LG(Mutex_);
    while(!StopReadingThread_){
        void* buff=ReadBuffer_.data()+BytesReaded_;
        size_t bytesToRead=ReadBuffer_.size()-BytesReaded_;
        LG.unlock();//nobody else is calling ReceiveData so its safe
        size_t bytes = Socket_TCP::ReceiveData(buff,bytesToRead);
        if(StopReadingThread_) continue;
        LG.lock();
        if(bytes)
            OnReceiveData(nullptr, 0);
        else return;
    }

}
