
#pragma once

#include "user.hpp"

namespace client {

class RoomHandler : public UserHandler {
public:
    RoomHandler() = default;
    ~RoomHandler() = default;
    RoomHandler(RoomHandler const &) = delete;
    RoomHandler &operator=(RoomHandler const &) = delete;
    RoomHandler(RoomHandler &&) noexcept = delete;
    RoomHandler &operator=(RoomHandler &&) noexcept = delete;

    void JoinRoom(const char *room_name, const char *password);
    virtual void LeaveRoom();

    [[nodiscard]] inline bool IsInRoom() const;
    [[nodiscard]] inline shared::Room GetRoom() const;

    void Logout() override;
    void Disconnect() override;

protected:
    void _OnEvent(EventData const &ev_data) override;

private:
    shared::Room room_;
    bool waiting_for_room_join_respond_ = false;

    void _LeaveRoom();
};

bool RoomHandler::IsInRoom() const { return room_.id_; }
shared::Room RoomHandler::GetRoom() const { return room_; }

}  // namespace client
