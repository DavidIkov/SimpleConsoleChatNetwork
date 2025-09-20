
#include "rooms_creation.hpp"

#include <cstring>

#include "client/rooms_creation.hpp"

namespace server {

RoomsCreationHandler::RoomsCreationHandler() {
    db_.exec(R"-(
            CREATE TABLE IF NOT EXISTS rooms (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            creator_id INTEGER NOT NULL,
            name VARCHAR()-" +
             std::to_string(shared::room_name_max_length) +
             R"-() NOT NULL UNIQUE,
            password VARCHAR()-" +
             std::to_string(shared::room_password_max_length) +
             R"-() NOT NULL,
            create_date DATETIME DEFAULT CURRENT_TIMESTAMP
            ))-");
}

RoomsCreationHandler::RoomDB_Record RoomsCreationHandler::GetRoomByID(
    shared::id_t id) {
    SQLite::Statement getting_room(db_,
                                   "SELECT creator_id, name, password, "
                                   "create_date FROM rooms WHERE id = ?");
    getting_room.bind(1, id);

    if (getting_room.executeStep()) {
        shared::id_t creator_id = getting_room.getColumn(0);
        std::string name = getting_room.getColumn(1);
        std::string password = getting_room.getColumn(2);
        std::string date = getting_room.getColumn(3);
        return {id, creator_id, std::move(name), std::move(password),
                std::move(date)};
    } else
        return {0};
}
RoomsCreationHandler::RoomDB_Record RoomsCreationHandler::GetRoomByName(
    const char* name) {
    SQLite::Statement getting_room(db_,
                                   "SELECT id, creator_id, password, "
                                   "create_date FROM rooms WHERE name = ?");
    getting_room.bind(1, name);

    if (getting_room.executeStep()) {
        shared::id_t id = getting_room.getColumn(0);
        shared::id_t creator_id = getting_room.getColumn(1);
        std::string password = getting_room.getColumn(2);
        std::string date = getting_room.getColumn(3);
        return {id, creator_id, std::move(name), std::move(password),
                std::move(date)};
    } else
        return {0};
}
RoomsCreationHandler::RoomCreatingResult RoomsCreationHandler::CreateRoom(
    shared::User const& creator, const char* name, const char* password) {
    if (!shared::CheckRoomNameSyntax(name))
        return {0, RoomCreatingResult::ResultType::IncorrectNameFormat};
    if (!shared::CheckRoomPasswordSyntax(password))
        return {0, RoomCreatingResult::ResultType::IncorrectPasswordFormat};
    if (GetRoomByName(name).id_)
        return {0, RoomCreatingResult::ResultType::NameAlreadyExists};

    SQLite::Statement insert_room(
        db_, "INSERT INTO rooms (creator_id, name, password) VALUES (?,?,?)");
    insert_room.bind(1, creator.id_);
    insert_room.bind(2, name);
    insert_room.bind(3, password);
    insert_room.exec();

    return {db_.getLastInsertRowid(),
            RoomCreatingResult::ResultType::AddedInDB};
}

std::unique_ptr<client::Base> RoomsCreationHandler::_ClientFactory(
    EventsHandler::ClientRawDescriptor desc) {
    return std::make_unique<client::RoomsCreationHandler>(this, desc);
}
}  // namespace server
