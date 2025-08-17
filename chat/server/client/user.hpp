#pragma once
#include "connection.hpp"

namespace client {
class UserHandler : public ConnectionHandler {
public:
    UserHandler(server::ConnectionsHandler *server, ClientRawDescriptor desc);
    ~UserHandler();
    UserHandler(UserHandler const &) = delete;
    UserHandler &operator=(UserHandler const &) = delete;
    UserHandler(UserHandler &&) noexcept = default;
    UserHandler &operator=(UserHandler &&) noexcept = default;

protected:
    void _OnEvent(events::Type evTyp, void const *evData) override;
private:
    shared::user_id_t id_ = 0;
    char username_[shared::username_max_length];
};
}  // namespace client
