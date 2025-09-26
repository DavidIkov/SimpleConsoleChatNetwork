#include "rooms.hpp"

#include <spdlog/spdlog.h>

using namespace events::data_types;

namespace client {

std::string RoomDB_Record::ToString() const {
    return fmt::format(
        "[ name: \"{}\", id: {}, creator id: {}, users amount: {}, creation "
        "date: {} ]",
        name_, id_, creator_id_, users_amount_, create_date_);
}

void RoomsHandler::JoinRoom(shared::id_t id, std::string_view password) {
    if (!shared::CheckRoomPasswordSyntax(password)) {
        SPDLOG_ERROR("Failed to join room. Incorrect password syntax. \"{}\".",
                     password);
        throw std::exception();
    }
    if (!IsLoggedIn()) {
        SPDLOG_ERROR("Failed to join room. Not logged in.");
        throw std::exception();
    }

    IncomingRespond join_room_resp = SendRequest(OutgoingRequest(
        "JoinRoom",
        {PacketData<INT64>(id), PacketData<STRING>(password.data())}));
    switch (join_room_resp.GetUInt32(0)) {
        case 0: {
            RoomDB_Record room_info = GetRoomDBRecordByID(id);
            if (!room_info.id_) {
                SPDLOG_ERROR(
                    "Failed to log in user. Could not get user database record "
                    "from server.");
                throw std::exception();
            }
            std::lock_guard LG(mutex_);
            SPDLOG_INFO("Joined room: {}.",
                        rooms_[room_info.id_] = std::move(room_info));
            break;
        }
        case 1:
            if (IsLoggedIn()) {
                SPDLOG_ERROR(
                    "Failed to join room. Server says that user is not "
                    "logged in. Client thinks the opposite. Possible desync.");
                throw std::exception();
            } else {
                SPDLOG_ERROR(
                    "Failed to join room. Server says that user is not "
                    "logged in. Client thinks the same. Why did client sent "
                    "request?");
                throw std::exception();
            }
        case 2:
            if (IsInRoom(id)) {
                SPDLOG_ERROR(
                    "Failed to join room. Server says that user is already "
                    "in this room. Client thinks the same. Why did client sent "
                    "request?");
                throw std::exception();
            } else {
                SPDLOG_ERROR(
                    "Failed to join room. Server says that user is already "
                    "in this room. Client thinks the opposite. Possible "
                    "desync.");
                throw std::exception();
            }
        case 3:
            SPDLOG_ERROR(
                "Failed to join room. Client thinks that password which was "
                "sent is correctly formatted, server thinks the opposite.");
            throw std::exception();
        case 4:
            SPDLOG_INFO("Failed to join room. Incorrect password.");
            break;
        case 5:
            SPDLOG_INFO("Failed to join room. ID does not exist.");
            break;
        default:
            SPDLOG_ERROR("Failed to join room. Unknown respond from server.");
            throw std::exception();
    }
}

void RoomsHandler::LeaveRoom(shared::id_t id) {
    bool in_this_room = IsInRoom(id);
    if (!in_this_room) {
        SPDLOG_ERROR("Failed to leave room. Not in this room.");
        throw std::exception();
    }

    IncomingRespond leave_room_resp =
        SendRequest(OutgoingRequest("LeaveRoom", {PacketData<UINT64>(id)}));

    switch (leave_room_resp.GetUInt32(0)) {
        case 0: {
            std::lock_guard LG(mutex_);
            auto room = rooms_.find(id);
            SPDLOG_INFO("Left room: {}.", room->second);
            rooms_.erase(room);
            break;
        }
        case 1:
            if (!in_this_room) {
                SPDLOG_ERROR(
                    "Failed to leave room. Server says that user is not "
                    "in this room. Client thinks the opposite. Possible "
                    "desync.");
                throw std::exception();
            } else {
                SPDLOG_ERROR(
                    "Failed to leave room. Server says that user is not "
                    "in this room. Client thinks the same. Why did client sent "
                    "request?");
                throw std::exception();
            }
        default:
            SPDLOG_ERROR("Failed to leave room. Unknown respond from server.");
            throw std::exception();
    }
}

void RoomsHandler::LeaveAllRooms() {
    std::unique_lock LG(mutex_);
    while (rooms_.size()) {
        shared::id_t id = rooms_.begin()->first;
        LG.unlock();
        LeaveRoom(id);
        LG.lock();
    }
}

void RoomsHandler::CreateRoom(std::string_view name,
                              std::string_view password) const {
    IncomingRespond create_resp = SendRequest(
        OutgoingRequest("CreateRoom", {PacketData<STRING>(name.data()),
                                       PacketData<STRING>(password.data())}));
    switch (create_resp.GetUInt32(0)) {
        case 0:
            SPDLOG_INFO("Created room with name: \"{}\" and password: \"{}\".",
                        name, password);
            break;
        case 1:
            SPDLOG_INFO("Failed to create room. Client is not logged in.");
            break;
        case 2:
            SPDLOG_INFO("Failed to create room. Name \"{}\" is already used.",
                        name);
            break;
        case 3:
            SPDLOG_ERROR(
                "Failed to create room. Client thinks that name which was "
                "sent is correctly formatted, server thinks the opposite.");
            throw std::exception();
        case 4:
            SPDLOG_ERROR(
                "Failed to create room. Client thinks that password which was "
                "sent is correctly formatted, server thinks the opposite.");
            throw std::exception();
        default:
            SPDLOG_ERROR("Failed to create room. Unknown respond from server.");
            throw std::exception();
    }
}

void RoomsHandler::GetRooms(
    std::function<void(const std::map<shared::id_t, RoomDB_Record> &)> const
        &callback) const {
    std::lock_guard LG(mutex_);
    callback(rooms_);
}

void RoomsHandler::GetRoom(
    shared::id_t id,
    std::function<void(const RoomDB_Record &)> const &callback) const {
    std::lock_guard LG(mutex_);
    callback(rooms_.at(id));
}

bool RoomsHandler::IsInRoom(shared::id_t id) const {
    std::lock_guard LG(mutex_);
    return rooms_.find(id) != rooms_.end();
}

void RoomsHandler::LogOutOfUser() {
    LeaveAllRooms();
    UserHandler::LogOutOfUser();
}

RoomDB_Record RoomsHandler::GetRoomDBRecordByID(shared::id_t id) const {
    IncomingRespond resp = SendRequest(
        OutgoingRequest("GetRoomDBRecord", {PacketData<INT64>(id)}));
    if (resp.GetUInt32(0) == 0) {
        RoomDB_Record rec;
        rec.id_ = id;
        rec.creator_id_ = resp.GetUInt64(1);
        rec.name_ = resp.GetString(2);
        rec.create_date_ = resp.GetString(3);
        rec.users_amount_ = resp.GetUInt32(4);
        return rec;
    } else
        return {0};
}
shared::id_t RoomsHandler::GetRoomIDByName(std::string_view name) const {
    IncomingRespond resp = SendRequest(
        OutgoingRequest("GetRoomIDByName", {PacketData<STRING>(name.data())}));
    if (resp.GetUInt32(0) == 0)
        return resp.GetUInt64(1);
    else
        return 0;
}

}  // namespace client
