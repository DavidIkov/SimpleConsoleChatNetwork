#include "parse.hpp"

#include <map>

#include "antlr4_parser/eventsBaseListener.h"
#include "antlr4_parser/eventsLexer.h"
#include "antlr4_parser/eventsParser.h"
#include "common.hpp"
#define EOF (-1)
#include "spdlog/spdlog.h"

namespace events {

using namespace data_types;

static std::map<std::string, EventDescription> events_description;
static std::map<event_type_id_t, decltype(events_description)::value_type>
    id_to_event_map;

class Parser : public eventsBaseListener {
public:
    void enterEvent_name(eventsParser::Event_nameContext* ctx) override {
        last_event_ = &(events_description[ctx->getText()] = {});
    }

    void enterRequest(eventsParser::RequestContext* ctx) override {
        in_request_ = true;
    }
    void enterRespond(eventsParser::RespondContext* ctx) override {
        in_request_ = false;
    }
    void enterStr_len(eventsParser::Str_lenContext* ctx) override {
        auto& desc =
            in_request_ ? last_event_->request_ : last_event_->respond_;
        auto str_field = TypeDescription<STRING>{std::stoul(ctx->getText())};
        desc.fields_.push_back(str_field);
        desc.max_size_ += str_field.GetMaxSize();
    }
    void enterEnum_(eventsParser::Enum_Context* ctx) override {
        auto& desc =
            in_request_ ? last_event_->request_ : last_event_->respond_;
        desc.fields_.push_back(TypeDescription<UINT32>());
        desc.max_size_ += TypeDescription<UINT32>().GetMaxSize();
    }
    void enterType(eventsParser::TypeContext* ctx) override {
        auto& desc =
            in_request_ ? last_event_->request_ : last_event_->respond_;
        if (ctx->getText() == "int32") {
            desc.fields_.push_back(TypeDescription<INT32>());
            desc.max_size_ += TypeDescription<INT32>().GetMaxSize();
        } else if (ctx->getText() == "int64") {
            desc.fields_.push_back(TypeDescription<INT64>());
            desc.max_size_ += TypeDescription<INT64>().GetMaxSize();
        } else if (ctx->getText() == "uint32") {
            desc.fields_.push_back(TypeDescription<UINT32>());
            desc.max_size_ += TypeDescription<UINT32>().GetMaxSize();
        } else if (ctx->getText() == "uint64") {
            desc.fields_.push_back(TypeDescription<UINT32>());
            desc.max_size_ += TypeDescription<UINT32>().GetMaxSize();
        } else if (ctx->getText() == "float") {
            desc.fields_.push_back(TypeDescription<FLOAT>());
            desc.max_size_ += TypeDescription<FLOAT>().GetMaxSize();
        } else if (ctx->getText() == "bool") {
            desc.fields_.push_back(TypeDescription<BOOL>());
            desc.max_size_ += TypeDescription<BOOL>().GetMaxSize();
        }
    }

private:
    bool in_request_ = false;
    EventDescription* last_event_ = nullptr;
};

void ParseEventsConfig(std::filesystem::path const& config) {
    try {
        std::ifstream stream(config);
        if (!stream.is_open()) {
            SPDLOG_ERROR(
                "Failed parsing events config. Could not open config file "
                "\"{}\".",
                config.c_str());
            throw std::exception();
        }

        antlr4::ANTLRInputStream input(stream);
        eventsLexer lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);
        eventsParser parser(&tokens);
        auto tree = parser.structure();
        Parser listener;
        antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);

        // TODO optimize, so it can be called multiple times
        {
            event_type_id_t id = 0;
            for (auto& ev : events_description) {
                if (id == std::numeric_limits<event_type_id_t>::max())
                    throw std::logic_error("too much possible events");
                ev.second.type_id_ = ++id;
                id_to_event_map.insert({ev.second.type_id_, ev});
            }
        }

    } catch (std::exception& err) {
        SPDLOG_ERROR(
            "Failed parsing events config. File name: \"{}\". Error: {}",
            config.c_str(), err.what());
        throw;
    }
}

EventDescription& GetEventDescriptionWithID(event_type_id_t id) {
    return id_to_event_map.at(id).second;
}

EventDescription& GetEventDescriptionWithName(std::string_view name) {
    // TODO why we acceping string view, and then making string out of it?
    return events_description.at(std::string(name));
}

std::string const& GetEventNameWithID(event_type_id_t id) {
    return id_to_event_map.at(id).first;
}

template <typename T>
static size_t PutInBuffer(void* buff, PacketData<T> const& field_data) {
    *(typename StoringType<T>::type*)buff = field_data.v_;
    return field_data.GetSize();
}

template <>
size_t PutInBuffer<STRING>(void* buff, PacketData<STRING> const& field_data) {
    std::memcpy(buff, field_data.v_.data(), field_data.GetSize());
    return field_data.GetSize();
}

Packet::Packet(EventDescription const& ev_desc, PacketType typ,
               std::initializer_list<PacketField> data)
    : desc_(typ == Request ? ev_desc.request_ : ev_desc.respond_),
      type_id_(ev_desc.type_id_) {
    offsets_.reserve(desc_.get().fields_.size());

    size_t size = 0;
    for (auto const& field : data)
        size += std::visit([](auto&& typ) { return typ.GetSize(); }, field);

    if (desc_.get().max_size_ < size) {
        SPDLOG_ERROR(
            "Packet is malformed. Size of packet, {} bytes, is bigger than max "
            "size defined by description, {} bytes.",
            data_.size(), desc_.get().max_size_);
        throw std::exception();
    }

    data_.resize(size);

    size_t off = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        auto const& field = *(data.begin() + i);
        size_t field_size = offsets_.emplace_back(std::visit(
            [&](auto&& typ) { return PutInBuffer(data_.data() + off, typ); },
            field));
        off += field_size;

        if (size_t field_max_size =
                std::visit([&](auto&& typ) { return typ.GetMaxSize(); },
                           desc_.get().fields_[i]);
            field_max_size < field_size) {
            SPDLOG_ERROR(
                "Packet is malformed. Size of field {} is too big, {} bytes, "
                "max size is {} bytes",
                i, field_size, field_max_size);
            throw std::exception();
        }
    }
    for (size_t i = data.size(); i < desc_.get().fields_.size(); ++i) {
        size_t field_size = offsets_.emplace_back(std::visit(
            [&](auto&& typ) {
                return PutInBuffer(
                    data_.data() + off,
                    PacketData<
                        typename std::remove_reference_t<decltype(typ)>::type>(
                        {}));
            },
            desc_.get().fields_[i]));
        off += field_size;
    }
}

template <typename T>
static void ProcessPacketField(TypeDescription<T> const& type_desc,
                               uint8_t const* data, size_t bytes,
                               Packet& pack) {
    size_t last_offset = pack.offsets_.size() ? *pack.offsets_.rbegin() : 0;
    size_t avail_bytes = bytes - last_offset;
    if (avail_bytes < sizeof(uint32_t)) {
        SPDLOG_ERROR(
            "Packet is malformed. Size of packet, {} bytes, is not enough.",
            bytes);
        throw std::exception();
    }
    pack.offsets_.emplace_back(last_offset + type_desc.GetMaxSize());
}

template <>
void ProcessPacketField<data_types::STRING>(
    TypeDescription<data_types::STRING> const& type_desc, uint8_t const* data,
    size_t bytes, Packet& pack) {
    size_t last_offset = pack.offsets_.size() ? *pack.offsets_.rbegin() : 0;
    size_t avail_bytes = bytes - last_offset;
    data += last_offset;

    size_t max_str_len = type_desc.GetMaxSize();
    size_t end_ind;
    for (end_ind = 0;
         data[end_ind] != 0 && end_ind < avail_bytes && end_ind < max_str_len;
         ++end_ind);
    if (end_ind == max_str_len) {
        SPDLOG_ERROR(
            "Packet is malformed. Failed parsing string, it is too long, max "
            "size is "
            "{} bytes.",
            max_str_len, end_ind);
        throw std::exception();
    } else if (end_ind == avail_bytes) {
        SPDLOG_ERROR(
            "Packet is malformed. Failed parsing string, could not find end of "
            "string in available bytes, {} bytes, either string dont have end, "
            "or packet is too short.",
            avail_bytes);
        throw std::exception();
    } else {
        ++end_ind;  // include null termination
        pack.offsets_.emplace_back(last_offset + end_ind);
    }
}

Packet::Packet(EventDescription const& ev_desc, PacketType typ,
               uint8_t const* data, size_t size)
    : desc_(typ == Request ? ev_desc.request_ : ev_desc.respond_),
      type_id_(ev_desc.type_id_) {
    data_.insert(data_.begin(), data, data + size);
    offsets_.reserve(desc_.get().fields_.size());

    if (desc_.get().max_size_ < size) {
        SPDLOG_ERROR(
            "Packet is malformed. Size of packet, {} bytes, is bigger than max "
            "size defined by description, {} bytes.",
            data_.size(), desc_.get().max_size_);
        throw std::exception();
    }

    for (auto& type_desc : desc_.get().fields_)
        std::visit(
            [&](auto&& typ) {
                return ProcessPacketField(typ, data, size, *this);
            },
            type_desc);

    if (offsets_.size() != desc_.get().fields_.size()) {
        SPDLOG_ERROR(
            "Packet is malformed. Amount of fields in packet, {},  does not "
            "equal amount of fields in packet desc_.get().iption, {}.",
            offsets_.size(), desc_.get().fields_.size());
        throw std::exception();
    }
}

std::string Packet::ToString() const {
    return fmt::format("[ id: {}, name: \"{}\", bytes: {} ]", type_id_,
                       events::GetEventNameWithID(type_id_), data_.size());
}

void const* Packet::GetVarOffset(size_t ind) const {
    return (data_.data() + (ind ? offsets_[ind - 1] : 0));
}
int32_t Packet::GetInt32(size_t ind) const {
    if (std::get_if<TypeDescription<INT32>>(&desc_.get().fields_[ind]))
        return *(int32_t const*)GetVarOffset(ind);
    else {
        SPDLOG_ERROR(
            "Failed to get int32 from packet. Description have different type "
            "at this place.");
        throw std::exception();
    }
}
int64_t Packet::GetInt64(size_t ind) const {
    if (std::get_if<TypeDescription<INT64>>(&desc_.get().fields_[ind]))
        return *(int64_t const*)GetVarOffset(ind);
    else {
        SPDLOG_ERROR(
            "Failed to get int64 from packet. Description have different type "
            "at this place.");
        throw std::exception();
    }
}
uint32_t Packet::GetUInt32(size_t ind) const {
    if (std::get_if<TypeDescription<UINT32>>(&desc_.get().fields_[ind]))
        return *(uint32_t const*)GetVarOffset(ind);
    else {
        SPDLOG_ERROR(
            "Failed to get uint32 from packet. Description have different type "
            "at this place.");
        throw std::exception();
    }
}
uint64_t Packet::GetUInt64(size_t ind) const {
    if (std::get_if<TypeDescription<UINT64>>(&desc_.get().fields_[ind]))
        return *(uint64_t const*)GetVarOffset(ind);
    else {
        SPDLOG_ERROR(
            "Failed to get uint64 from packet. Description have different type "
            "at this place.");
        throw std::exception();
    }
}
bool Packet::GetBool(size_t ind) const {
    if (std::get_if<TypeDescription<BOOL>>(&desc_.get().fields_[ind]))
        return *(bool const*)GetVarOffset(ind);
    else {
        SPDLOG_ERROR(
            "Failed to get bool from packet. Description have different type "
            "at this place.");
        throw std::exception();
    }
}
float Packet::GetFloat(size_t ind) const {
    if (std::get_if<TypeDescription<FLOAT>>(&desc_.get().fields_[ind]))
        return *(float const*)GetVarOffset(ind);
    else {
        SPDLOG_ERROR(
            "Failed to get float from packet. Description have different type "
            "at this place.");
        throw std::exception();
    }
}
std::string_view Packet::GetString(size_t ind) const {
    if (std::get_if<TypeDescription<STRING>>(&desc_.get().fields_[ind]))
        return (char const*)GetVarOffset(ind);
    else {
        SPDLOG_ERROR(
            "Failed to get string from packet. Description have different type "
            "at this place.");
        throw std::exception();
    }
}

}  // namespace events
