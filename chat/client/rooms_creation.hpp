
#pragma once

#include "user.hpp"

namespace client {

class RoomsCreationHandler : public UserHandler {
public:
    RoomsCreationHandler() = default;
    ~RoomsCreationHandler() = default;
    RoomsCreationHandler(RoomsCreationHandler const &) = delete;
    RoomsCreationHandler &operator=(RoomsCreationHandler const &) = delete;
    RoomsCreationHandler(RoomsCreationHandler &&) noexcept = delete;
    RoomsCreationHandler &operator=(RoomsCreationHandler &&) noexcept = delete;

    void CreateRoom(const char *name, const char *password);

protected:
    void _OnEvent(EventData const &ev_data) override;

private:
    bool waiting_for_room_creation_respond_ = false;

};

}  // namespace client
