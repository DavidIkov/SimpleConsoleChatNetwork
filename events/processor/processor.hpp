#pragma once
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <map>
#include <optional>
#include <thread>

#include "events/parser/common.hpp"
#include "networking/socket/tcp.hpp"

namespace events {
class EventsProcessor : private networking::socket::TCP {
public:
    /*
     *   When server accepts connection, it may return raw descriptor, to use
     * this descriptor, use this struct. This is the only scenario where this
     * struct should be used. Dont use this with just raw descriptors that were
     *  aquired in any other scenario.
     */
    struct ClientRawDescriptor {
        networking::RawDescriptorT desc_;
        inline ClientRawDescriptor(networking::RawDescriptorT desc)
            : desc_(desc) {}
    };

    EventsProcessor();
    EventsProcessor(ClientRawDescriptor desc);
    virtual ~EventsProcessor();
    EventsProcessor(EventsProcessor const &) = delete;
    EventsProcessor &operator=(EventsProcessor const &) = delete;
    EventsProcessor(EventsProcessor &&) noexcept;
    EventsProcessor &operator=(EventsProcessor &&) noexcept;

    void Connect(const networking::Endpoint &endp);
    virtual void Disconnect();

    using networking::socket::TCP::GetLocalAddress;
    using networking::socket::TCP::GetRemoteAddress;
    using networking::socket::TCP::IsConnected;

    struct OutgoingRequest : Packet {
        OutgoingRequest(std::string_view name,
                        std::initializer_list<PacketField> const &data);
    };
    struct IncomingRespond : Packet {
        IncomingRespond(event_type_id_t type_id, uint8_t const *data,
                        size_t size);
    };
    struct OutgoingRespond : Packet {
        OutgoingRespond(std::string_view name,
                        std::initializer_list<PacketField> const &data);
    };
    struct IncomingRequest : Packet {
        IncomingRequest(event_type_id_t type_id, uint8_t const *data,
                        size_t size);
    };

    [[nodiscard]] IncomingRespond SendRequest(
        OutgoingRequest const &pack) const;

    void WaitTillEndOfDataProcessing();

protected:
    virtual OutgoingRespond _ProcessRequest(IncomingRequest const &pack);

private:
    std::condition_variable end_of_processing_cv_;
    bool end_of_processing_ = false;

    // id of 0 is invalid
    using packet_id_t = uint32_t;
    packet_id_t _GetNewPacketID() const;
    mutable packet_id_t packet_id_counter_ = 0;

    struct WaitingRespond {
        std::condition_variable cv_;
        bool done_ = false;
        std::optional<IncomingRespond> data_;
    };

    mutable std::map<packet_id_t, std::unique_ptr<WaitingRespond>>
        active_requests_;
    mutable std::mutex mutex_;

    void _ReadingThreadFunc();
    std::thread reading_thread_;

    std::atomic_bool stopping_reading_thread_ = false;
};

}  // namespace events

template <>
struct fmt::formatter<events::EventsProcessor::IncomingRequest>
    : fmt::formatter<std::string> {
    inline auto format(const events::EventsProcessor::IncomingRequest &pack,
                       fmt::format_context &ctx) const {
        return fmt::formatter<std::string>::format(pack.ToString(), ctx);
    }
};

inline std::ostream &operator<<(
    std::ostream &stream,
    const events::EventsProcessor::IncomingRequest &packet) {
    return stream << packet.ToString();
}

template <>
struct fmt::formatter<events::EventsProcessor::IncomingRespond>
    : fmt::formatter<std::string> {
    inline auto format(const events::EventsProcessor::IncomingRespond &pack,
                       fmt::format_context &ctx) const {
        return fmt::formatter<std::string>::format(pack.ToString(), ctx);
    }
};

inline std::ostream &operator<<(
    std::ostream &stream,
    const events::EventsProcessor::IncomingRespond &packet) {
    return stream << packet.ToString();
}

template <>
struct fmt::formatter<events::EventsProcessor::OutgoingRequest>
    : fmt::formatter<std::string> {
    inline auto format(const events::EventsProcessor::OutgoingRequest &pack,
                       fmt::format_context &ctx) const {
        return fmt::formatter<std::string>::format(pack.ToString(), ctx);
    }
};

inline std::ostream &operator<<(
    std::ostream &stream,
    const events::EventsProcessor::OutgoingRequest &packet) {
    return stream << packet.ToString();
}

template <>
struct fmt::formatter<events::EventsProcessor::OutgoingRespond>
    : fmt::formatter<std::string> {
    inline auto format(const events::EventsProcessor::OutgoingRespond &pack,
                       fmt::format_context &ctx) const {
        return fmt::formatter<std::string>::format(pack.ToString(), ctx);
    }
};

inline std::ostream &operator<<(
    std::ostream &stream,
    const events::EventsProcessor::OutgoingRespond &packet) {
    return stream << packet.ToString();
}
