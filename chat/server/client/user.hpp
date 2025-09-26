#pragma once
#include "chat/server/users.hpp"
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

    [[nodiscard]] bool IsLoggedIn() const;

    void GetUser(std::function<void(const server::UserDB_Record &)> const
                     &callback) const;

    void Disconnect() override;

    virtual void LogOutOfUser();

protected:
    OutgoingRespond _ProcessRequest(IncomingRequest const &pack) override;

private:
    mutable std::mutex mutex_;

    server::UserDB_Record user_;
};

}  // namespace client
