#pragma once
#include "spdlog/fmt/bundled/format.h"
#include "users.hpp"

namespace server {

struct RoomDB_Record {
    shared::id_t id_ = 0, creator_id_;
    std::string name_, password_, create_date_;
    uint32_t users_amount_;

    std::string ToString() const;
};

class RoomsHandler : public UsersHandler {
public:
    RoomsHandler();

    [[nodiscard]] RoomDB_Record GetRoomByID(shared::id_t id);
    [[nodiscard]] shared::id_t GetRoomIDByName(std::string_view name);
    [[nodiscard]] bool RoomIDExists(shared::id_t id);

    struct CreatingRoomResult {
        shared::id_t id_;
        enum class ResultType : uint8_t {
            NoErrors,
            NameAlreadyUsed,
            IncorrectNameFormat,
            IncorrectPasswordFormat,
        } result_;
    };
    [[nodiscard]] CreatingRoomResult CreateRoom(shared::id_t creator_id,
                                                std::string_view name,
                                                std::string_view password);

    [[nodiscard]] bool IsUserInRoom(shared::id_t room_id, shared::id_t user_id);

    enum class AddingUserToRoomResult : uint8_t {
        NoErrors,
        RoomIDDoesNotExist,
        UserIDDoesNotExist,
        UserAlreadyInRoom
    };
    [[nodiscard]] AddingUserToRoomResult AddUserToRoom(shared::id_t room_id,
                                                       shared::id_t user_id);

    enum class RemovingUserFromRoomResult : uint8_t {
        NoErrors,
        RoomIDDoesNotExist,
        UserIDDoesNotExist,
        UserIsNotInRoom
    };
    [[nodiscard]] RemovingUserFromRoomResult RemoveUserFromRoom(
        shared::id_t room_id, shared::id_t user_id);

protected:
    std::unique_ptr<client::Base> _ClientFactory(
        events::EventsProcessor::ClientRawDescriptor desc) override;
};

}  // namespace server

template <>
struct fmt::formatter<server::RoomDB_Record> : fmt::formatter<std::string> {
    auto format(const server::RoomDB_Record& record,
                fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(record.ToString(), ctx);
    }
};

inline std::ostream& operator<<(std::ostream& stream,
                                const server::RoomDB_Record& record) {
    return stream << record.ToString();
}
