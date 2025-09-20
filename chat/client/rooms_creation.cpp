#include "rooms_creation.hpp"

#include <cstring>
#include <iostream>

namespace client {

void RoomsCreationHandler::CreateRoom(const char *name, const char *password) {
    if (!IsLoggedIn()) throw std::logic_error("not logged in");

    if (!shared::CheckRoomNameSyntax(name))
        throw std::logic_error("incorrect room name syntax");
    if (!shared::CheckRoomPasswordSyntax(name))
        throw std::logic_error("incorrect password syntax");

    if (waiting_for_room_creation_respond_)
        throw std::logic_error("already waiting for room creation respond");

    events::CreateRoomAttempEvent create_room_attemp;
    std::strcpy(create_room_attemp.name_, name);
    std::strcpy(create_room_attemp.password_, password);
    SendEvent(create_room_attemp);

    waiting_for_room_creation_respond_ = true;

    std::unique_lock LG(mutex_, std::adopt_lock);
    event_respond_cv_.wait(
        LG, [&]() -> bool { return !waiting_for_room_creation_respond_; });
    LG.release();
}

void RoomsCreationHandler::_OnEvent(EventData const &ev_data) {
    if (ev_data.type_ == events::Type::CreateRoomAttempRespond) {
        if (!waiting_for_room_creation_respond_) {
            std::cout
                << "got create room attemp respond, while not waiting for it"
                << std::endl;
            return;
        }
        auto const &respond_data =
            *(events::CreateRoomAttempRespondEvent *)ev_data.data_;
        if (respond_data.id_) {
            switch (respond_data.response_) {
                case events::CreateRoomAttempRespondEvent::RespondType::Created:
                    std::cout << "created room" << std::endl;
                    break;
                default:
                    std::cout << "created room, unknown respond" << std::endl;
            }

        } else {
            switch (respond_data.response_) {
                case events::CreateRoomAttempRespondEvent::RespondType::
                    IncorrectNameFormat:
                    std::cout << "incorrect room name format" << std::endl;
                    break;
                case events::CreateRoomAttempRespondEvent::RespondType::
                    IncorrectPasswordFormat:
                    std::cout << "incorrect password format" << std::endl;
                    break;
                case events::CreateRoomAttempRespondEvent::RespondType::
                    NameAlreadyExists:
                    std::cout << "name already exists" << std::endl;
                    break;
                case events::CreateRoomAttempRespondEvent::RespondType::
                    NotLoggedIn:
                    std::cout << "not logged in" << std::endl;
                    break;
                case events::CreateRoomAttempRespondEvent::RespondType::Unknown:
                    std::cout << "unknown error" << std::endl;
                    break;
                default:
                    std::cout << "failed to join, unknown respond" << std::endl;
            }
        }

        waiting_for_room_creation_respond_ = false;
        event_respond_cv_.notify_all();

    } else
        UserHandler::_OnEvent(ev_data);
}

}  // namespace client
