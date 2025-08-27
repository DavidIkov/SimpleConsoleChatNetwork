
#include "rooms.hpp"

#include <algorithm>
#include <cstring>

#include "client/room.hpp"

namespace server {

const RoomsHandler::RoomDB_Record* RoomsHandler::GetRoomFromDB_FromID(
    shared::user_id_t id) {
    auto iter =
        std::find_if(database_.begin(), database_.end(),
                     [id](RoomDB_Record const& rec) { return rec.id_ == id; });
    if (iter == database_.end())
        return nullptr;
    else
        return &*iter;
}
const RoomsHandler::RoomDB_Record* RoomsHandler::GetRoomFromDB_FromName(
    const char* name) {
    auto iter = std::find_if(database_.begin(), database_.end(),
                             [name](RoomDB_Record const& rec) {
                                 return !std::strcmp(rec.name_, name);
                             });
    if (iter == database_.end())
        return nullptr;
    else
        return &*iter;
}
RoomsHandler::RoomAddingResult RoomsHandler::AddRoomToDB(const char* name,
                                                         const char* password) {
    if (!shared::CheckRoomNameSyntax(name))
        return {0, RoomAddingResult::ResultType::IncorrectNameFormat};
    if (!shared::CheckRoomPasswordSyntax(password))
        return {0, RoomAddingResult::ResultType::IncorrectPasswordFormat};

    RoomDB_Record& rec = database_.emplace_back();
    rec.id_ = ++id_counter_;
    std::strcpy(rec.name_, name);
    std::strcpy(rec.password_, password);
    return {rec.id_, RoomAddingResult::ResultType::AddedInDB};
}

std::unique_ptr<client::Base> RoomsHandler::_ConnectionFactory(
    EventsHandler::ClientRawDescriptor desc) {
    return std::make_unique<client::RoomHandler>(this, desc);
}
}  // namespace server
