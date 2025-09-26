#pragma once
#include "chat/shared.hpp"
#include "connections.hpp"
#include "spdlog/fmt/bundled/format.h"

namespace server {

struct UserDB_Record {
    shared::id_t id_ = 0;
    bool online_;
    std::string name_, password_, create_date_;

    std::string ToString() const;
};

class UsersHandler : public ConnectionsHandler {
public:
    UsersHandler();

    [[nodiscard]] UserDB_Record GetUserByID(shared::id_t id);
    [[nodiscard]] shared::id_t GetUserIDByName(std::string_view name);
    [[nodiscard]] bool UserIDExists(shared::id_t id);

    enum class MarkUserResult { NoErrors, IDDoesNotExist };
    [[nodiscard]] MarkUserResult MarkUserAsOnline(shared::id_t id);
    [[nodiscard]] MarkUserResult MarkUserAsOffline(shared::id_t id);

    struct UserRegisteringResult {
        shared::id_t id_;
        enum class ResultType : uint8_t {
            NoErrors,
            NameAlreadyUsed,
            IncorrectNameFormat,
            IncorrectPasswordFormat,
        } result_;
    };
    [[nodiscard]] UserRegisteringResult RegisterUser(std::string_view name,
                                                     std::string_view password);

protected:
    std::unique_ptr<client::Base> _ClientFactory(
        events::EventsProcessor::ClientRawDescriptor desc) override;
};

}  // namespace server

template <>
struct fmt::formatter<server::UserDB_Record> : fmt::formatter<std::string> {
    auto format(const server::UserDB_Record& record,
                fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(record.ToString(), ctx);
    }
};

inline std::ostream& operator<<(std::ostream& stream,
                                const server::UserDB_Record& record) {
    return stream << record.ToString();
}
