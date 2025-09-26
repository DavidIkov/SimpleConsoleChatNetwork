
#include "rooms.hpp"

#include "client/rooms.hpp"
#include "spdlog/spdlog.h"

namespace server {

std::string RoomDB_Record::ToString() const {
    return fmt::format(
        "[ name: \"{}\", id: {}, creator id: {}, password: \"{}\", online: {}, "
        "creation date: "
        "{}, users amount: {} ]",
        name_, id_, creator_id_, password_, create_date_, users_amount_);
}

RoomsHandler::RoomsHandler() {
    GetDB([](SQLite::Database& db) {
        try {
            db.exec(R"-(
            CREATE TABLE IF NOT EXISTS rooms (
            id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
            creator_id INTEGER NOT NULL,
            name TEXT NOT NULL UNIQUE,
            password TEXT NOT NULL,
            create_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
            users_amount INTEGER NOT NULL
            ))-");
        } catch (std::exception& err) {
            SPDLOG_ERROR("Failed to initialize rooms table. Error: {}",
                         err.what());
            throw;
        }
        try {
            db.exec(R"-(
            CREATE TABLE IF NOT EXISTS rooms_users (
            room_id INTEGER NUT NULL,
            user_id INTEGER NOT NULL,
            PRIMARY KEY (room_id, user_id)
            ))-");
        } catch (std::exception& err) {
            SPDLOG_ERROR("Failed to initialize rooms_users table. Error: {}",
                         err.what());
            throw;
        }
    });
}

RoomDB_Record RoomsHandler::GetRoomByID(shared::id_t id) {
    RoomDB_Record record{0};
    GetDB([&](SQLite::Database& db) {
        try {
            SQLite::Statement getting_user(
                db,
                "SELECT creator_id, name, password, "
                "create_date, users_amount FROM rooms WHERE id = ?");
            getting_user.bind(1, (int64_t)id);

            if (getting_user.executeStep()) {
                shared::id_t creator_id = getting_user.getColumn(0).getInt64();
                std::string name = getting_user.getColumn(1);
                std::string password = getting_user.getColumn(2);
                std::string date = getting_user.getColumn(3);
                uint32_t users_amount = getting_user.getColumn(4);
                record = {id,
                          creator_id,
                          std::move(name),
                          std::move(password),
                          std::move(date),
                          users_amount};
            }
        } catch (std::exception& err) {
            SPDLOG_ERROR("Failed to get room by id. Error: {}.", err.what());
            throw;
        }
    });
    return record;
}
shared::id_t RoomsHandler::GetRoomIDByName(std::string_view name) {
    shared::id_t ret_id = 0;
    GetDB([&](SQLite::Database& db) {
        try {
            SQLite::Statement getting_user(
                db, "SELECT id FROM rooms WHERE name = ?");
            getting_user.bind(1, name.data());

            if (getting_user.executeStep()) {
                shared::id_t id = getting_user.getColumn(0).getInt64();
                ret_id = id;
            }
        } catch (std::exception& err) {
            SPDLOG_ERROR("Failed to get room id by name. Error: {}",
                         err.what());
            throw;
        }
    });
    return ret_id;
}

bool RoomsHandler::RoomIDExists(shared::id_t id) {
    bool exists = false;
    GetDB([&](SQLite::Database& db) {
        try {
            SQLite::Statement check_room(db,
                                         "SELECT 1 FROM rooms WHERE id = ?");
            check_room.bind(1, (int64_t)id);
            if (check_room.executeStep()) {
                exists = true;
            }
        } catch (std::exception& err) {
            SPDLOG_ERROR("Failed to check if room id exists. Error: {}",
                         err.what());
            throw;
        }
    });
    return exists;
}

RoomsHandler::CreatingRoomResult RoomsHandler::CreateRoom(
    shared::id_t creator_id, std::string_view name, std::string_view password) {
    if (!shared::CheckRoomNameSyntax(name))
        return {0, CreatingRoomResult::ResultType::IncorrectNameFormat};
    if (!shared::CheckRoomPasswordSyntax(password))
        return {0, CreatingRoomResult::ResultType::IncorrectPasswordFormat};
    if (GetRoomIDByName(name))
        return {0, CreatingRoomResult::ResultType::NameAlreadyUsed};

    shared::id_t id;
    GetDB([&](SQLite::Database& db) {
        try {
            SQLite::Statement insert_user(
                db,
                "INSERT INTO rooms (creator_id, name, password, users_amount) "
                "VALUES (?, ?, ?, ?)");
            insert_user.bind(1, (int64_t)creator_id);
            insert_user.bind(2, name.data());
            insert_user.bind(3, password.data());
            insert_user.bind(4, 0);
            insert_user.exec();
            id = db.getLastInsertRowid();
        } catch (std::exception& err) {
            SPDLOG_ERROR("Failed to create room. Error: {}", err.what());
            throw;
        }
    });

    SPDLOG_INFO("Added new room \"{}\" to database.", name);

    return {id, CreatingRoomResult::ResultType::NoErrors};
}

bool RoomsHandler::IsUserInRoom(shared::id_t room_id, shared::id_t user_id) {
    if (!RoomIDExists(room_id)) return false;
    if (!UserIDExists(user_id)) return false;

    bool res = false;

    GetDB([&](SQLite::Database& db) {
        try {
            SQLite::Statement add_user(
                db, "SELECT 1 FROM rooms_users WHERE room_id = ? AND user_id = ?");
            add_user.bind(1, (int64_t)room_id);
            add_user.bind(2, (int64_t)user_id);
            if (add_user.executeStep()) res = true;
        } catch (std::exception& err) {
            SPDLOG_ERROR("Failed to check if user is in room. Error: {}",
                         err.what());
            throw;
        }
    });
    return res;
}

auto RoomsHandler::AddUserToRoom(shared::id_t room_id, shared::id_t user_id)
    -> AddingUserToRoomResult {
    if (!RoomIDExists(room_id))
        return AddingUserToRoomResult::RoomIDDoesNotExist;
    if (!UserIDExists(user_id))
        return AddingUserToRoomResult::UserIDDoesNotExist;
    if (IsUserInRoom(room_id, user_id))
        return AddingUserToRoomResult::UserAlreadyInRoom;

    uint32_t users_amount = GetRoomByID(room_id).users_amount_;

    GetDB([&](SQLite::Database& db) {
        try {
            {
                SQLite::Statement add_user(
                    db, "UPDATE rooms SET users_amount = ? WHERE id = ?");
                add_user.bind(1, users_amount + 1);
                add_user.bind(2, (int64_t)room_id);
                add_user.exec();
            }
            {
                SQLite::Statement add_user(
                    db,
                    "INSERT INTO rooms_users (room_id, user_id) VALUES (?, ?)");
                add_user.bind(1, (int64_t)room_id);
                add_user.bind(2, (int64_t)user_id);
                add_user.exec();
            }
        } catch (std::exception& err) {
            SPDLOG_ERROR("Failed to add user to room. Error: {}", err.what());
            throw;
        }
    });

    SPDLOG_INFO("Added user {} to room {} in database.", user_id, room_id);

    return AddingUserToRoomResult::NoErrors;
}

auto RoomsHandler::RemoveUserFromRoom(
    shared::id_t room_id, shared::id_t user_id) -> RemovingUserFromRoomResult {
    if (!RoomIDExists(room_id))
        return RemovingUserFromRoomResult::RoomIDDoesNotExist;
    if (!UserIDExists(user_id))
        return RemovingUserFromRoomResult::UserIDDoesNotExist;
    if (!IsUserInRoom(room_id, user_id))
        return RemovingUserFromRoomResult::UserIsNotInRoom;

    uint32_t users_amount = GetRoomByID(room_id).users_amount_;

    GetDB([&](SQLite::Database& db) {
        try {
            {
                SQLite::Statement add_user(
                    db, "UPDATE rooms SET users_amount = ? WHERE id = ?");
                add_user.bind(1, users_amount - 1);
                add_user.bind(2, (int64_t)room_id);
                add_user.exec();
            }
            {
                SQLite::Statement add_user(
                    db,
                    "DELETE FROM rooms_users WHERE room_id = ? AND user_id=?");
                add_user.bind(1, (int64_t)room_id);
                add_user.bind(2, (int64_t)user_id);
                add_user.exec();
            }
        } catch (std::exception& err) {
            SPDLOG_ERROR("Failed to remove user from room. Error: {}",
                         err.what());
            throw;
        }
    });

    SPDLOG_INFO("Removed user {} from room {} in database.", user_id, room_id);

    return RemovingUserFromRoomResult::NoErrors;
}

std::unique_ptr<client::Base> RoomsHandler::_ClientFactory(
    events::EventsProcessor::ClientRawDescriptor desc) {
    return std::make_unique<client::RoomsHandler>(this, desc);
}
}  // namespace server
