
#pragma once

#include <functional>

#include "chat/shared.hpp"
#include "connection.hpp"

namespace client {

struct UserDB_Record {
    shared::id_t id_ = 0;
    bool online_;
    std::string name_, create_date_;

    std::string ToString() const;
};

class UserHandler : public ConnectionHandler {
public:
    UserHandler() = default;
    ~UserHandler() = default;
    UserHandler(UserHandler const &) = delete;
    UserHandler &operator=(UserHandler const &) = delete;
    UserHandler(UserHandler &&) noexcept = delete;
    UserHandler &operator=(UserHandler &&) noexcept = delete;

    void LogInUser(shared::id_t id, std::string_view password);
    virtual void LogOutOfUser();
    void RegisterUser(std::string_view name, std::string_view password) const;

    [[nodiscard]] bool IsLoggedIn() const;
    void GetUser(
        std::function<void(const UserDB_Record &)> const &callback) const;

    void Disconnect() override;

    [[nodiscard]] UserDB_Record GetUserDBRecordByID(shared::id_t id) const;
    [[nodiscard]] shared::id_t GetUserIDByName(std::string_view name) const;

private:
    UserDB_Record user_;
    mutable std::mutex mutex_;
};

}  // namespace client

template <>
struct fmt::formatter<client::UserDB_Record> : fmt::formatter<std::string> {
    auto format(const client::UserDB_Record &record,
                fmt::format_context &ctx) const {
        return fmt::formatter<std::string>::format(record.ToString(), ctx);
    }
};

inline std::ostream &operator<<(std::ostream &stream,
                                const client::UserDB_Record &record) {
    return stream << record.ToString();
}
