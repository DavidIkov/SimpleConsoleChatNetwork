#pragma once
#include "users.hpp"

namespace server {
class RoomsCreationHandler : public UsersHandler {
public:
    RoomsCreationHandler();

    struct RoomDB_Record {
        shared::id_t id_;
        shared::id_t creator_id_t;
        std::string name_, password_, create_date_;
    };

    [[nodiscard]] RoomDB_Record GetRoomByID(shared::id_t id);
    [[nodiscard]] RoomDB_Record GetRoomByName(const char* name);

    struct RoomCreatingResult {
        shared::id_t id_;
        enum class ResultType : uint8_t {
            AddedInDB,
            IncorrectNameFormat,
            IncorrectPasswordFormat,
            NameAlreadyExists
        } result_;
    };
    [[nodiscard]] RoomCreatingResult CreateRoom(shared::User const& creator,
                                                const char* name,
                                                const char* password);

protected:
    virtual std::unique_ptr<client::Base> _ClientFactory(
        EventsHandler::ClientRawDescriptor desc);
};
}  // namespace server
