
#pragma once

#include "user.hpp"

namespace client {

struct RoomDB_Record {
    shared::id_t id_ = 0, creator_id_;
    std::string name_, create_date_;
    uint32_t users_amount_;

    std::string ToString() const;
};

class RoomsHandler : public UserHandler {
public:
    RoomsHandler() = default;
    ~RoomsHandler() = default;
    RoomsHandler(RoomsHandler const &) = delete;
    RoomsHandler &operator=(RoomsHandler const &) = delete;
    RoomsHandler(RoomsHandler &&) noexcept = delete;
    RoomsHandler &operator=(RoomsHandler &&) noexcept = delete;

    void JoinRoom(shared::id_t id, std::string_view password);
    virtual void LeaveRoom(shared::id_t id);
    void LeaveAllRooms();
    void CreateRoom(std::string_view name, std::string_view password) const;

    void GetRooms(
        std::function<void(const std::map<shared::id_t, RoomDB_Record> &)> const
            &callback) const;
    void GetRoom(
        shared::id_t id,
        std::function<void(const RoomDB_Record &)> const &callback) const;

    [[nodiscard]] bool IsInRoom(shared::id_t id) const;

    void LogOutOfUser() override;

    [[nodiscard]] RoomDB_Record GetRoomDBRecordByID(shared::id_t id) const;
    [[nodiscard]] shared::id_t GetRoomIDByName(std::string_view name) const;

private:
    std::map<shared::id_t, RoomDB_Record> rooms_;
    mutable std::mutex mutex_;
};

}  // namespace client

template <>
struct fmt::formatter<client::RoomDB_Record> : fmt::formatter<std::string> {
    auto format(const client::RoomDB_Record &record,
                fmt::format_context &ctx) const {
        return fmt::formatter<std::string>::format(record.ToString(), ctx);
    }
};

inline std::ostream &operator<<(std::ostream &stream,
                                const client::RoomDB_Record &record) {
    return stream << record.ToString();
}
