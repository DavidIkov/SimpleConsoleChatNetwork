#pragma once
#include "users.hpp"

namespace server {
class RoomsHandler : public UsersHandler {
public:
    struct RoomDB_Record {
        shared::room_id_t id_;
        char name_[shared::room_name_max_length];
        char password_[shared::room_password_max_length];
    };

    [[nodiscard]] const RoomDB_Record* GetRoomFromDB_FromID(
        shared::room_id_t id);
    [[nodiscard]] const RoomDB_Record* GetRoomFromDB_FromName(const char* name);

    struct RoomAddingResult {
        shared::room_id_t id_;
        enum class ResultType : uint8_t {
            AddedInDB,
            IncorrectNameFormat,
            IncorrectPasswordFormat,
        } result_;
    };
    [[nodiscard]] RoomAddingResult AddRoomToDB(const char* name,
                                               const char* password);

    void AddClientToRoom(shared::room_id_t room_id, client::Base& client);
    void RemoveClientFromRoom(shared::room_id_t room_id, client::Base& client);

protected:
    virtual std::unique_ptr<client::Base> _ConnectionFactory(
        EventsHandler::ClientRawDescriptor desc);

private:
    uint32_t id_counter_ = 0;
    // TEMPORARY, TODO IMPLEMENT DATABASE
    std::vector<RoomDB_Record> database_;
};
}  // namespace server
