
#pragma once

#include "connection.hpp"

namespace client {

class UserHandler : public ConnectionHandler {
public:
    UserHandler() = default;
    ~UserHandler() = default;
    UserHandler(UserHandler const &) = delete;
    UserHandler &operator=(UserHandler const &) = delete;
    UserHandler(UserHandler &&) noexcept = delete;
    UserHandler &operator=(UserHandler &&) noexcept = delete;

    void Login(const char *name, const char *password);
    void Logout();

    [[nodiscard]] inline bool IsLoggedIn() const;
    [[nodiscard]] inline shared::User GetUser() const;

    void Disconnect() override;

protected:
    void _OnEvent(EventData const &ev_data) override;
    void _OnDisconnect() override;

    virtual void _OnLogOut();

private:
    shared::User user_;
    bool waiting_for_login_respond_ = false;
};

bool UserHandler::IsLoggedIn() const { return user_.id_; }
shared::User UserHandler::GetUser() const { return user_; }

}  // namespace client
