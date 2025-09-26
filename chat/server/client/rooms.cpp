
#include "rooms.hpp"

#include "chat/server/rooms.hpp"
#include "spdlog/spdlog.h"

namespace client {

RoomsHandler::RoomsHandler(server::Base *server, ClientRawDescriptor desc)
    : UserHandler(server, desc) {}

bool RoomsHandler::IsInRoom(shared::id_t id) const {
    std::lock_guard LG(mutex_);
    return rooms_.find(id) != rooms_.end();
}
void RoomsHandler::GetRooms(
    std::function<void(const std::map<shared::id_t, server::RoomDB_Record>
                           &)> const &callback) const {
    std::lock_guard LG(mutex_);
    callback(rooms_);
}
void RoomsHandler::GetRoom(
    shared::id_t id,
    std::function<void(const server::RoomDB_Record &)> const &callback) const {
    std::lock_guard LG(mutex_);
    callback(rooms_.at(id));
}

void RoomsHandler::LeaveRoom(shared::id_t id) {
    if (!IsInRoom(id)) {
        SPDLOG_ERROR("Failed to leave room. Not in this room");
        throw std::exception();
    }
    std::lock_guard LG(mutex_);
    auto room = rooms_.find(id);
    SPDLOG_INFO("Left room: {}.", room->second);
    rooms_.erase(room);
}

void RoomsHandler::LogOutOfUser() {
    LeaveAllRooms();
    UserHandler::LogOutOfUser();
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

auto RoomsHandler::_ProcessRequest(IncomingRequest const &pack)
    -> OutgoingRespond {
    server::RoomsHandler &server = *(server::RoomsHandler *)server_;
    if (pack.type_id_ ==
        events::GetEventDescriptionWithName("GetRoomIDByName").type_id_) {
        if (!shared::CheckRoomNameSyntax(pack.GetString(0))) {
            return OutgoingRespond(
                "GetRoomIDByName",
                {events::data_types::PacketData<events::data_types::UINT32>(2),
                 events::data_types::PacketData<events::data_types::UINT64>(
                     0)});
        }
        shared::id_t room_id = server.GetRoomIDByName(pack.GetString(0));
        if (!room_id) {
            return OutgoingRespond(
                "GetRoomIDByName",
                {events::data_types::PacketData<events::data_types::UINT32>(1),
                 events::data_types::PacketData<events::data_types::UINT64>(
                     0)});
        }
        return OutgoingRespond(
            "GetRoomIDByName",
            {events::data_types::PacketData<events::data_types::UINT32>(0),
             events::data_types::PacketData<events::data_types::UINT64>(
                 room_id)});
    } else if (pack.type_id_ ==
               events::GetEventDescriptionWithName("GetRoomDBRecord")
                   .type_id_) {
        server::RoomDB_Record rec = server.GetRoomByID(pack.GetUInt64(0));
        if (!rec.id_) {
            return OutgoingRespond(
                "GetRoomDBRecord",
                {events::data_types::PacketData<events::data_types::UINT32>(1),
                 events::data_types::PacketData<events::data_types::UINT64>(0),
                 events::data_types::PacketData<events::data_types::STRING>(""),
                 events::data_types::PacketData<events::data_types::STRING>(""),
                 events::data_types::PacketData<events::data_types::UINT32>(
                     0)});
        }
        return OutgoingRespond(
            "GetRoomDBRecord",
            {events::data_types::PacketData<events::data_types::UINT32>(0),
             events::data_types::PacketData<events::data_types::UINT64>(
                 rec.creator_id_),
             events::data_types::PacketData<events::data_types::STRING>(
                 rec.name_),
             events::data_types::PacketData<events::data_types::STRING>(
                 rec.create_date_),
             events::data_types::PacketData<events::data_types::UINT32>(
                 rec.users_amount_)});
    } else if (pack.type_id_ ==
               events::GetEventDescriptionWithName("CreateRoom").type_id_) {
        if (!IsLoggedIn()) {
            return OutgoingRespond(
                "CreateRoom",
                {events::data_types::PacketData<events::data_types::UINT32>(1),
                 events::data_types::PacketData<events::data_types::UINT64>(
                     0)});
        }

        shared::id_t usr_id;
        GetUser([&](const server::UserDB_Record &user) { usr_id = user.id_; });

        server::RoomsHandler::CreatingRoomResult res =
            server.CreateRoom(usr_id, pack.GetString(0), pack.GetString(1));
        switch (res.result_) {
            case server::RoomsHandler::CreatingRoomResult::ResultType::NoErrors:
                return OutgoingRespond(
                    "CreateRoom",
                    {events::data_types::PacketData<events::data_types::UINT32>(
                         0),
                     events::data_types::PacketData<events::data_types::UINT64>(
                         res.id_)});
            case server::RoomsHandler::CreatingRoomResult::ResultType::
                IncorrectNameFormat:
                return OutgoingRespond(
                    "CreateRoom",
                    {events::data_types::PacketData<events::data_types::UINT32>(
                        3)});
            case server::RoomsHandler::CreatingRoomResult::ResultType::
                IncorrectPasswordFormat:
                return OutgoingRespond(
                    "CreateRoom",
                    {events::data_types::PacketData<events::data_types::UINT32>(
                        4)});
            case server::RoomsHandler::CreatingRoomResult::ResultType::
                NameAlreadyUsed:
                return OutgoingRespond(
                    "CreateRoom",
                    {events::data_types::PacketData<events::data_types::UINT32>(
                        2)});
            default:
                SPDLOG_WARN("Creating room resulted in unknown response.");
                return OutgoingRespond(
                    "CreateRoom",
                    {events::data_types::PacketData<events::data_types::UINT32>(
                        5)});
        }
    } else if (pack.type_id_ ==
               events::GetEventDescriptionWithName("JoinRoom").type_id_) {
        if (!IsLoggedIn())
            return OutgoingRespond(
                "JoinRoom",
                {events::data_types::PacketData<events::data_types::UINT32>(
                    1)});

        if (IsInRoom(pack.GetUInt64(0)))
            return OutgoingRespond(
                "JoinRoom",
                {events::data_types::PacketData<events::data_types::UINT32>(
                    2)});

        if (!shared::CheckRoomPasswordSyntax(pack.GetString(1))) {
            return OutgoingRespond(
                "JoinRoom",
                {events::data_types::PacketData<events::data_types::UINT32>(
                    3)});
        }

        server::RoomDB_Record rec = server.GetRoomByID(pack.GetUInt64(0));
        if (!rec.id_) {
            return OutgoingRespond(
                "JoinRoom",
                {events::data_types::PacketData<events::data_types::UINT32>(
                    5)});
        }
        if (rec.password_ != pack.GetString(1)) {
            return OutgoingRespond(
                "JoinRoom",
                {events::data_types::PacketData<events::data_types::UINT32>(
                    4)});
        }

        shared::id_t usr_id;
        GetUser([&](const server::UserDB_Record &user) { usr_id = user.id_; });

        server::RoomsHandler::AddingUserToRoomResult add_res =
            server.AddUserToRoom(rec.id_, usr_id);
        switch (add_res) {
            case server::RoomsHandler::AddingUserToRoomResult::NoErrors:
                break;
            case server::RoomsHandler::AddingUserToRoomResult::
                RoomIDDoesNotExist:
                SPDLOG_ERROR(
                    "Failed to add user to room. Room ID does not exist, but "
                    "we "
                    "just checked that this id should exist.");
                throw std::exception();
            case server::RoomsHandler::AddingUserToRoomResult::
                UserIDDoesNotExist:
                SPDLOG_ERROR(
                    "Failed to add user to room. User ID does not exist, but "
                    "we "
                    "just checked that this id should exist.");
                throw std::exception();
            case server::RoomsHandler::AddingUserToRoomResult::
                UserAlreadyInRoom:
                SPDLOG_ERROR(
                    "Failed to add user to room. User is already in room, and "
                    "server client thinks the opposite, probably a bug.");
                throw std::exception();
            default: {
                SPDLOG_WARN("Adding user to room resulted in unknown respond.");
                return OutgoingRespond(
                    "JoinRoom",
                    {events::data_types::PacketData<events::data_types::UINT32>(
                        6)});
            }
        }

        {
            std::lock_guard LG(mutex_);
            rooms_[rec.id_] = std::move(rec);
        }

        return OutgoingRespond(
            "JoinRoom",
            {events::data_types::PacketData<events::data_types::UINT32>(0)});
    } else if (pack.type_id_ ==
               events::GetEventDescriptionWithName("LeaveRoom").type_id_) {
        if (!IsInRoom(pack.GetUInt64(0))) {
            return OutgoingRespond(
                "LeaveRoom",
                {events::data_types::PacketData<events::data_types::UINT32>(
                    1)});
        }

        shared::id_t usr_id;
        GetUser([&](const server::UserDB_Record &user) { usr_id = user.id_; });

        server::RoomsHandler::RemovingUserFromRoomResult rem_res =
            server.RemoveUserFromRoom(pack.GetUInt64(0), usr_id);
        switch (rem_res) {
            case server::RoomsHandler::RemovingUserFromRoomResult::NoErrors:
                break;
            case server::RoomsHandler::RemovingUserFromRoomResult::
                RoomIDDoesNotExist:
                SPDLOG_ERROR(
                    "Failed to leave room. Room ID does not exist, "
                    "but we just checked that this id should exist.");
                throw std::exception();
            case server::RoomsHandler::RemovingUserFromRoomResult::
                UserIDDoesNotExist:
                SPDLOG_ERROR(
                    "Failed to leave room. User ID does not exist, "
                    "but we just checked that this id should exist.");
                throw std::exception();
            default: {
                SPDLOG_WARN("Leaving room resulted in unknown respond.");
                return OutgoingRespond(
                    "LeaveRoom",
                    {events::data_types::PacketData<events::data_types::UINT32>(
                        2)});
            }
        }

        {
            std::lock_guard LG(mutex_);
            rooms_.erase(rooms_.find(pack.GetUInt64(0)));
        }

        return OutgoingRespond(
            "LeaveRoom",
            {events::data_types::PacketData<events::data_types::UINT32>(0)});
    } else
        return UserHandler::_ProcessRequest(pack);
}

}  // namespace client
