#include "room.hpp"

#include <cstring>
#include <iostream>

#include "chat/server/rooms.hpp"

namespace client {

RoomHandler::RoomHandler(server::Base *server, ClientRawDescriptor desc)
    : UserHandler(server, desc) {}

RoomHandler::~RoomHandler(){
    //TODO
}

void RoomHandler::_OnEvent(EventData const &ev_data) {
    if (ev_data.type_ == events::Type::JoinRoomAttemp) {
        auto &joinData = *(events::JoinRoomAttempEvent *)ev_data.data_;

        if (!IsLoggedIn()) {
            SendEvent(events::JoinRoomAttempRespondEvent{
                events::JoinRoomAttempRespondEvent::RespondType::NotLoggedIn,
                0});
            return;
        }

        if (room_.id_) {
            SendEvent(events::JoinRoomAttempRespondEvent{
                events::JoinRoomAttempRespondEvent::RespondType::AlreadyInRoom,
                0});
            return;
        }

        server::RoomsHandler &server = *(server::RoomsHandler *)server_;

        auto LG = server.AquireLock();

        if (server::RoomsHandler::RoomDB_Record const *found_record =
                server.GetRoomFromDB_FromName(joinData.name_)) {
            if (std::strcmp(found_record->password_, joinData.password_)) {
                std::cout << GetUser() << " tried joining room "
                          << joinData.name_ << " with incorrect password"
                          << std::endl;

                SendEvent(events::JoinRoomAttempRespondEvent{
                    events::JoinRoomAttempRespondEvent::RespondType::
                        WrongPassword,
                    0});
            } else {
                std::cout << GetUser() << " joined " << joinData.name_
                          << std::endl;

                SendEvent(events::JoinRoomAttempRespondEvent{
                    events::JoinRoomAttempRespondEvent::RespondType::Joined,
                    found_record->id_});

                room_.id_ = found_record->id_;
                std::memcpy(room_.name_, joinData.name_,
                            std::strlen(joinData.name_));
            }
        } else {
            server::RoomsHandler::RoomAddingResult res =
                server.AddRoomToDB(joinData.name_, joinData.password_);
            if (res.id_) {
                std::cout << GetUser() << " created a new room called "
                          << joinData.name_ << std::endl;

                SendEvent(events::JoinRoomAttempRespondEvent{
                    events::JoinRoomAttempRespondEvent::RespondType::
                        JoinedNewRoom,
                    res.id_});

                room_.id_ = res.id_;
                std::memcpy(room_.name_, joinData.name_,
                            std::strlen(joinData.name_));

            } else {
                switch (res.result_) {
                    case server::RoomsHandler::RoomAddingResult::ResultType::
                        IncorrectPasswordFormat: {
                        std::cout << GetUser()
                                  << " sent malformed password, no null "
                                     "termination. strange."
                                  << std::endl;

                        SendEvent(events::JoinRoomAttempRespondEvent{
                            events::JoinRoomAttempRespondEvent::RespondType::
                                IncorrectPasswordFormat,
                            0});
                        break;
                    }
                    case server::RoomsHandler::RoomAddingResult::ResultType::
                        IncorrectNameFormat: {
                        std::cout << GetUser()
                                  << " sent malformed name, no null "
                                     "termination. strange."
                                  << std::endl;

                        SendEvent(events::JoinRoomAttempRespondEvent{
                            events::JoinRoomAttempRespondEvent::RespondType::
                                IncorrectNameFormat,
                            0});
                        break;
                    }
                    default: {
                        std::cout << GetUser() << "'s attemp to join "
                                  << joinData.name_
                                  << " resulted in unknown respond"
                                  << std::endl;

                        SendEvent(events::JoinRoomAttempRespondEvent{
                            events::JoinRoomAttempRespondEvent::RespondType::
                                Unknown,
                            0});
                    }
                }
            }
        }
    } else if (ev_data.type_ == events::Type::LeaveRoom) {
        if (!IsLoggedIn())
            std::cout << GetRemoteAddress()
                      << " tried to leave room while not logged in"
                      << std::endl;
        else if (IsInRoom()) {
            std::cout << GetUser() << " left room " << GetRoom() << std::endl;
            room_.id_ = 0;
        } else
            std::cout << GetUser() << " tried to leave room while not in room"
                      << std::endl;
    } else
        UserHandler::_OnEvent(ev_data);
}

void RoomHandler::_OnLogOut() {
    if (IsInRoom()) {
        std::cout << GetUser() << " left room " << room_ << " cause of log out "
                  << std::endl;
        room_.id_ = 0;
    }
    UserHandler::_OnLogOut();
}

}  // namespace client
