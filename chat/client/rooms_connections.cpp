#include "room.hpp"

#include <cstring>
#include <iostream>

namespace client {

void RoomHandler::JoinRoom(const char *room_name, const char *password) {
    if (!IsLoggedIn()) throw std::logic_error("not logged in");

    if (!shared::CheckRoomNameSyntax(room_name))
        throw std::logic_error("incorrect room name syntax");
    if (!shared::CheckRoomPasswordSyntax(room_name))
        throw std::logic_error("incorrect password syntax");

    if (waiting_for_room_join_respond_)
        throw std::logic_error("already waiting for join room respond");
    if (IsInRoom()) throw std::logic_error("already in room");

    std::strcpy(room_.name_, room_name);

    events::JoinRoomAttempEvent join_room_attemp;
    std::strcpy(join_room_attemp.name_, room_name);
    std::strcpy(join_room_attemp.password_, password);
    SendEvent(join_room_attemp);

    waiting_for_room_join_respond_ = true;

    std::unique_lock LG(mutex_, std::adopt_lock);
    event_respond_cv_.wait(
        LG, [&]() -> bool { return !waiting_for_room_join_respond_; });
    LG.release();
}

void RoomHandler::LeaveRoom() {
    if (!IsLoggedIn()) throw std::logic_error("not logged in");
    if (!IsInRoom()) throw std::logic_error("not in room");

    _LeaveRoom();
    SendEvent(events::LeaveRoomEvent{});
}

void RoomHandler::_LeaveRoom() {
    std::cout << "left from room " << room_ << std::endl;
    room_.id_ = 0;
}

void RoomHandler::Disconnect() {
    if (IsInRoom()) _LeaveRoom();
    UserHandler::Disconnect();
}

void RoomHandler::Logout() {
    if (IsInRoom()) _LeaveRoom();
    UserHandler::Logout();
}

void RoomHandler::_OnEvent(EventData const &ev_data) {
    if (ev_data.type_ == events::Type::JoinRoomAttempRespond) {
        if (!waiting_for_room_join_respond_) {
            std::cout
                << "got join room attemp respond, while not waiting for it"
                << std::endl;
            return;
        }
        auto const &respond_data =
            *(events::JoinRoomAttempRespondEvent *)ev_data.data_;
        if (respond_data.id_) {
            room_.id_ = respond_data.id_;
            switch (respond_data.response_) {
                case events::JoinRoomAttempRespondEvent::RespondType::Joined:
                    std::cout << "joined room " << room_ << std::endl;
                    break;
                case events::JoinRoomAttempRespondEvent::RespondType::
                    JoinedNewRoom:
                    std::cout << "joined and created room " << room_
                              << std::endl;
                    break;
                default:
                    std::cout << "joined room, unknown respond" << std::endl;
            }

        } else {
            switch (respond_data.response_) {
                case events::JoinRoomAttempRespondEvent::RespondType::
                    AlreadyInRoom:
                    std::cout << "user is already in room" << std::endl;
                    break;
                case events::JoinRoomAttempRespondEvent::RespondType::
                    IncorrectNameFormat:
                    std::cout << "incorrect room name format" << std::endl;
                    break;
                case events::JoinRoomAttempRespondEvent::RespondType::
                    IncorrectPasswordFormat:
                    std::cout << "incorrect password format" << std::endl;
                    break;
                case events::JoinRoomAttempRespondEvent::RespondType::
                    WrongPassword:
                    std::cout << "incorrect password" << std::endl;
                    break;
                case events::JoinRoomAttempRespondEvent::RespondType::
                    NotLoggedIn:
                    std::cout << "not logged in" << std::endl;
                    break;
                case events::JoinRoomAttempRespondEvent::RespondType::Unknown:
                    std::cout << "unknown error" << std::endl;
                    break;
                default:
                    std::cout << "failed to join, unknown respond" << std::endl;
            }
        }

        waiting_for_room_join_respond_ = false;
        event_respond_cv_.notify_all();

    } else
        UserHandler::_OnEvent(ev_data);
}

}  // namespace client
