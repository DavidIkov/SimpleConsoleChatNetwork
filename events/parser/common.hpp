#pragma once
#include <string>
#include <variant>
#include <vector>

#include "data_types.hpp"

#include "spdlog/fmt/bundled/format.h"

namespace events {

using event_type_id_t = std::size_t;

using PacketDescriptionField =
    std::variant<data_types::TypeDescription<data_types::INT32>,
                 data_types::TypeDescription<data_types::INT64>,
                 data_types::TypeDescription<data_types::UINT32>,
                 data_types::TypeDescription<data_types::UINT64>,
                 data_types::TypeDescription<data_types::STRING>,
                 data_types::TypeDescription<data_types::FLOAT>,
                 data_types::TypeDescription<data_types::BOOL>>;

struct PacketDescription {
    std::vector<PacketDescriptionField> fields_;
    size_t max_size_ = 0;
};

struct EventDescription {
    event_type_id_t type_id_ = 0;
    PacketDescription request_, respond_;
};

EventDescription& GetEventDescriptionWithID(event_type_id_t id);
EventDescription& GetEventDescriptionWithName(std::string_view name);
std::string const& GetEventNameWithID(event_type_id_t id);

using PacketField = std::variant<data_types::PacketData<data_types::INT32>,
                                 data_types::PacketData<data_types::INT64>,
                                 data_types::PacketData<data_types::UINT32>,
                                 data_types::PacketData<data_types::UINT64>,
                                 data_types::PacketData<data_types::STRING>,
                                 data_types::PacketData<data_types::FLOAT>,
                                 data_types::PacketData<data_types::BOOL>>;

struct Packet {
    enum PacketType : uint8_t { Request, Respond };
    std::reference_wrapper<const PacketDescription> desc_;
    event_type_id_t type_id_ = 0;
    std::vector<uint8_t> data_;
    std::vector<size_t> offsets_;

    Packet(EventDescription const& ev_desc, PacketType typ,
           std::initializer_list<PacketField> data);
    Packet(EventDescription const& ev_desc, PacketType typ, uint8_t const* data,
           size_t size);

    void const* GetVarOffset(size_t ind) const;
    int32_t GetInt32(size_t ind) const;
    int64_t GetInt64(size_t ind) const;
    uint32_t GetUInt32(size_t ind) const;
    uint64_t GetUInt64(size_t ind) const;
    bool GetBool(size_t ind) const;
    float GetFloat(size_t ind) const;
    std::string_view GetString(size_t ind) const;

    std::string ToString() const;
};
}  // namespace events

template <>
struct fmt::formatter<events::Packet> : fmt::formatter<std::string> {
    auto format(const events::Packet& packet, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(packet.ToString(), ctx);
    }
};

inline std::ostream& operator<<(std::ostream& stream,
                                const events::Packet& packet) {
    return stream << packet.ToString();
}
