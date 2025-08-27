#pragma once
#include "connection.hpp"

namespace client {
class UserHandler : public ConnectionHandler {
public:
    UserHandler(server::Base *server, ClientRawDescriptor desc);
    ~UserHandler() = default;
    UserHandler(UserHandler const &) = delete;
    UserHandler &operator=(UserHandler const &) = delete;
    UserHandler(UserHandler &&) noexcept = delete;
    UserHandler &operator=(UserHandler &&) noexcept = delete;

    [[nodiscard]] inline bool IsLoggedIn() const;
    [[nodiscard]] inline shared::User GetUser() const;

protected:
    void _OnEvent(EventData const &ev_data) override;

    virtual void _OnLogOut();

private:
    shared::User user_;
};

bool UserHandler::IsLoggedIn() const { return user_.id_; }
shared::User UserHandler::GetUser() const { return user_; }

}  // namespace client
