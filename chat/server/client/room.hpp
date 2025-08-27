#pragma once
#include "user.hpp"

namespace client {
class RoomHandler : public UserHandler {
public:
    RoomHandler(server::Base *server, ClientRawDescriptor desc);
    ~RoomHandler();
    RoomHandler(RoomHandler const &) = delete;
    RoomHandler &operator=(RoomHandler const &) = delete;
    RoomHandler(RoomHandler &&) noexcept = delete;
    RoomHandler &operator=(RoomHandler &&) noexcept = delete;

    [[nodiscard]] inline bool IsInRoom() const;
    [[nodiscard]] inline shared::Room GetRoom() const;

protected:
    void _OnEvent(EventData const &ev_data) override;

    void _OnLogOut() override;

private:
    shared::Room room_;
};

bool RoomHandler::IsInRoom() const { return room_.id_; }
shared::Room RoomHandler::GetRoom() const { return room_; }

}  // namespace client
