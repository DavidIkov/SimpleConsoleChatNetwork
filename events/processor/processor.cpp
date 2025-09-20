#include "processor.hpp"

#include "spdlog/spdlog.h"

namespace events {

EventsProcessor::OutgoingRequest::OutgoingRequest(
    std::string_view name, std::initializer_list<PacketField> const &data)
    : Packet(GetEventDescriptionWithName(name), Request, data) {}
EventsProcessor::IncomingRespond::IncomingRespond(event_type_id_t type_id,
                                                  uint8_t const *data,
                                                  size_t size)
    : Packet(GetEventDescriptionWithID(type_id), Respond, data, size) {}
EventsProcessor::OutgoingRespond::OutgoingRespond(
    std::string_view name, std::initializer_list<PacketField> const &data)
    : Packet(GetEventDescriptionWithName(name), Respond, data) {}

EventsProcessor::IncomingRequest::IncomingRequest(event_type_id_t type_id,
                                                  uint8_t const *data,
                                                  size_t size)
    : Packet(GetEventDescriptionWithID(type_id), Request, data, size) {}

EventsProcessor::EventsProcessor() {}

EventsProcessor::EventsProcessor(ClientRawDescriptor desc)
    : TCP(desc.desc_),
      reading_thread_(std::thread([this]() { _ReadingThreadFunc(); })) {}

EventsProcessor::~EventsProcessor() {
    if (IsConnected()) Disconnect();
    if (reading_thread_.joinable()) reading_thread_.join();
}

void EventsProcessor::WaitTillEndOfDataProcessing() {
    std::unique_lock LG(mutex_);
    end_of_processing_cv_.wait(LG,
                               [&]() -> bool { return end_of_processing_; });
}

void EventsProcessor::Connect(const networking::Endpoint &endp) {
    if (IsConnected()) {
        SPDLOG_ERROR(
            "Failed to connect socket to {}, socket is already connected.",
            endp);
        throw std::exception();
    }
    TCP::Open();
    TCP::Connect(endp);
    stopping_reading_thread_ = false;
    std::lock_guard LG(mutex_);
    end_of_processing_ = false;
    if (reading_thread_.joinable()) reading_thread_.join();
    reading_thread_ = std::thread([this]() { _ReadingThreadFunc(); });
}
void EventsProcessor::Disconnect() {
    std::lock_guard LG(mutex_);
    if (reading_thread_.joinable() &&
        reading_thread_.get_id() != std::this_thread::get_id()) {
        stopping_reading_thread_ = true;
        TCP::ShutdownReading();
        reading_thread_.join();
    }
    TCP::Close();
    for (auto &req : active_requests_) req.second->cv_.notify_one();
    end_of_processing_ = true;
    end_of_processing_cv_.notify_all();
}

auto EventsProcessor::SendRequest(OutgoingRequest const &ev) const
    -> IncomingRespond {
    if (!IsConnected()) {
        SPDLOG_ERROR("Failed to send request. Socket is not connected.");
        throw std::exception();
    }
    std::unique_lock LG(mutex_);
    packet_id_t req_id = _GetNewPacketID();

    if (active_requests_.find(req_id) != active_requests_.end()) {
        SPDLOG_ERROR("Failed to send request. Too much active requests.");
        throw std::exception();
    }
    auto &req = active_requests_[req_id] = std::make_unique<WaitingRespond>();

    SPDLOG_INFO("Sent request {}.", ev);

    event_type_id_t converted_id = ev.type_id_ * 2;
    TCP::SendData(&converted_id, sizeof(converted_id));
    size_t packet_size = ev.data_.size();
    TCP::SendData(&packet_size, sizeof(packet_size));
    TCP::SendData(ev.data_.data(), ev.data_.size());
    TCP::SendData(&req_id, sizeof(req_id));

    req->cv_.wait(LG, [&]() -> bool { return req->done_ || !IsConnected(); });

    if (!IsConnected()) {
        SPDLOG_ERROR(
            "Failed to get request. Disconnected while waiting for respond.");
        throw std::exception();
    }

    IncomingRespond resp = std::move(req->data_.value());

    SPDLOG_INFO("Received response {}.", resp);

    active_requests_.erase(req_id);
    return resp;
}

auto EventsProcessor::_GetNewPacketID() const -> packet_id_t {
    if (packet_id_counter_ == std::numeric_limits<packet_id_t>::max())
        packet_id_counter_ = 0;
    return ++packet_id_counter_;
}

void EventsProcessor::_ReadingThreadFunc() {
    try {
        while (1) {
            enum {
                Processed,
                ParsingError,
                NoActiveRequest,
                Disconnected
            } packet_processing_result = Processed;

            bool cur_packet_is_a_respond_;
            const EventDescription *ev_desc;
            const PacketDescription *pack_desc;

            event_type_id_t ev_type_id;
            std::vector<uint8_t> ev_data;

            {  // get event type id
                size_t bytes_left = sizeof(event_type_id_t);
                event_type_id_t converted_type_id;
                while (bytes_left && packet_processing_result == Processed) {
                    size_t bytes = TCP::ReceiveData(
                        (uint8_t *)&converted_type_id +
                            sizeof(converted_type_id) - bytes_left,
                        bytes_left);
                    if (bytes)
                        bytes_left -= bytes;
                    else
                        packet_processing_result = Disconnected;
                }
                if (packet_processing_result == Processed) {
                    ev_type_id = converted_type_id / 2;
                    cur_packet_is_a_respond_ = converted_type_id % 2;
                    ev_desc = &GetEventDescriptionWithID(ev_type_id);
                    pack_desc = cur_packet_is_a_respond_ ? &ev_desc->respond_
                                                         : &ev_desc->request_;
                }
            }

            {  // get packet size
                size_t bytes_left = sizeof(size_t);
                size_t packet_len;
                while (bytes_left && packet_processing_result == Processed) {
                    size_t bytes =
                        TCP::ReceiveData((uint8_t *)&packet_len +
                                             sizeof(packet_len) - bytes_left,
                                         bytes_left);
                    if (bytes)
                        bytes_left -= bytes;
                    else
                        packet_processing_result = Disconnected;
                }
                if (packet_processing_result == Processed) {
                    if (packet_len > pack_desc->max_size_)
                        packet_processing_result = ParsingError;
                    else
                        ev_data.resize(packet_len);
                }
            }

            {  // get actual packet
                size_t bytes_left = ev_data.size();
                while (bytes_left && packet_processing_result == Processed) {
                    size_t bytes = TCP::ReceiveData(
                        ev_data.data() + ev_data.size() - bytes_left,
                        bytes_left);
                    if (bytes)
                        bytes_left -= bytes;
                    else
                        packet_processing_result = Disconnected;
                }
            }

            {  // get request/respond id
                size_t bytes_left = sizeof(packet_id_t);
                packet_id_t packet_id;
                while (bytes_left && packet_processing_result == Processed) {
                    size_t bytes = TCP::ReceiveData(
                        (uint8_t *)&packet_id + sizeof(packet_id) - bytes_left,
                        bytes_left);
                    if (bytes)
                        bytes_left -= bytes;
                    else
                        packet_processing_result = Disconnected;
                }
                if (packet_processing_result == Processed) {
                    if (cur_packet_is_a_respond_) {
                        std::lock_guard LG(mutex_);
                        if (auto active_req = active_requests_.find(packet_id);
                            active_req == active_requests_.end()) {
                            packet_processing_result = NoActiveRequest;
                        } else {
                            active_req->second->done_ = true;
                            active_req->second->data_ = IncomingRespond(
                                ev_type_id, ev_data.data(), ev_data.size());
                            active_req->second->cv_.notify_one();
                        }
                    } else {
                        IncomingRequest req(ev_type_id, ev_data.data(),
                                            ev_data.size());
                        SPDLOG_INFO("Received request {}.", req);
                        OutgoingRespond resp = _ProcessRequest(req);
                        SPDLOG_INFO("Responded with {}.", resp);
                        event_type_id_t converted_id = resp.type_id_ * 2 + 1;
                        TCP::SendData(&converted_id, sizeof(converted_id));
                        size_t packet_size = resp.data_.size();
                        TCP::SendData(&packet_size, sizeof(packet_size));
                        TCP::SendData(resp.data_.data(), resp.data_.size());
                        TCP::SendData(&packet_id, sizeof(packet_id));
                    }
                }
            }
            if (packet_processing_result == Processed)
                ;
            else if (packet_processing_result == Disconnected) {
                if (stopping_reading_thread_) {
                    return;
                } else {
                    SPDLOG_INFO(
                        "Stopped reading thread. Other side disconnected.");
                    Disconnect();
                    return;
                }
            } else if (packet_processing_result == ParsingError) {
                SPDLOG_WARN("Parsing error in reading thread, disconneting.");
                Disconnect();
                return;
            } else if (packet_processing_result == NoActiveRequest) {
                SPDLOG_WARN("Received response with no active request.");
            } else {
                SPDLOG_WARN(
                    "Unknown result of processing packet in reading thread, "
                    "disconnecting.");
                Disconnect();
                return;
            }
        }
    } catch (std::exception &err) {
        SPDLOG_ERROR("Caught exception in reading thread, disconneting. {}",
                     err.what());
        if (IsConnected()) Disconnect();
    }
}

auto EventsProcessor::_ProcessRequest(IncomingRequest const &ev)
    -> OutgoingRespond {
    SPDLOG_ERROR("Failed to process request respond, no one handled it. {}.",
                 ev);
    throw std::exception();
}

}  // namespace events
