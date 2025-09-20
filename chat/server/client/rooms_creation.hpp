#pragma once
#include "user.hpp"

namespace client {
class RoomsCreationHandler : public UserHandler {
public:
    RoomsCreationHandler(server::Base *server, ClientRawDescriptor desc);
    ~RoomsCreationHandler() = default;
    RoomsCreationHandler(RoomsCreationHandler const &) = delete;
    RoomsCreationHandler &operator=(RoomsCreationHandler const &) = delete;
    RoomsCreationHandler(RoomsCreationHandler &&) noexcept = delete;
    RoomsCreationHandler &operator=(RoomsCreationHandler &&) noexcept = delete;

protected:
    void _OnEvent(EventData const &ev_data) override;
};
}  // namespace client
