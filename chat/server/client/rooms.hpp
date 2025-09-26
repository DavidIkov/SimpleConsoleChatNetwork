#pragma once
#include "chat/server/rooms.hpp"
#include "user.hpp"

namespace client {
class RoomsHandler : public UserHandler {
public:
    RoomsHandler(server::Base *server, ClientRawDescriptor desc);
    ~RoomsHandler() = default;
    RoomsHandler(RoomsHandler const &) = delete;
    RoomsHandler &operator=(RoomsHandler const &) = delete;
    RoomsHandler(RoomsHandler &&) noexcept = delete;
    RoomsHandler &operator=(RoomsHandler &&) noexcept = delete;

    [[nodiscard]] bool IsInRoom(shared::id_t id) const;

    void GetRooms(
        std::function<void(const std::map<shared::id_t, server::RoomDB_Record>
                               &)> const &callback) const;
    void GetRoom(shared::id_t id,
                 std::function<void(const server::RoomDB_Record &)> const
                     &callback) const;

    void LeaveAllRooms();
    virtual void LeaveRoom(shared::id_t id);

    void LogOutOfUser() override;

protected:
    OutgoingRespond _ProcessRequest(IncomingRequest const &pack) override;

private:
    mutable std::mutex mutex_;

    std::map<shared::id_t, server::RoomDB_Record> rooms_;
};

}  // namespace client
