#pragma once
#include "user.hpp"

namespace client {
class RoomHandler : public UserHandler {
public:
    RoomHandler(server::Base *server, ClientRawDescriptor desc);
    ~RoomHandler() = default;
    RoomHandler(RoomHandler const &) = delete;
    RoomHandler &operator=(RoomHandler const &) = delete;
    RoomHandler(RoomHandler &&) noexcept = delete;
    RoomHandler &operator=(RoomHandler &&) noexcept = delete;

    [[nodiscard]] inline bool IsInRoom() const;
    [[nodiscard]] inline shared::Room GetRoom() const;

    virtual void LeaveRoom();

    void Disconnect() override;
    void Logout() override;

protected:
    void _OnEvent(EventData const &ev_data) override;

private:
    shared::Room room_;

    void _LeaveRoom();
};

bool RoomHandler::IsInRoom() const { return room_.id_; }
shared::Room RoomHandler::GetRoom() const { return room_; }

}  // namespace client
