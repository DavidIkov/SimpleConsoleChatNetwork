#include "rooms_creation.hpp"

#include <cstring>
#include <iostream>

#include "chat/server/rooms_creation.hpp"

namespace client {

RoomsCreationHandler::RoomsCreationHandler(server::Base *server,
                                           ClientRawDescriptor desc)
    : UserHandler(server, desc) {}

void RoomsCreationHandler::_OnEvent(EventData const &ev_data) {
    if (ev_data.type_ == events::Type::CreateRoomAttemp) {
        if (!IsLoggedIn()) {
            std::cout << GetRemoteAddress()
                      << " tried creating room while not logged in"
                      << std::endl;
            SendEvent(events::CreateRoomAttempRespondEvent{
                events::CreateRoomAttempRespondEvent::RespondType::NotLoggedIn,
                0});
            return;
        }

        auto &roomData = *(events::CreateRoomAttempEvent *)ev_data.data_;

        server::RoomsCreationHandler &server =
            *(server::RoomsCreationHandler *)server_;

        auto LG = server.AquireLock();

        server::RoomsCreationHandler::RoomCreatingResult res =
            server.CreateRoom(GetUser(), roomData.name_, roomData.password_);
        if (res.id_) {
            std::cout << GetUser() << " created a new room called "
                      << roomData.name_ << std::endl;

            SendEvent(events::CreateRoomAttempRespondEvent{
                events::CreateRoomAttempRespondEvent::RespondType::Created,
                res.id_});
        } else {
            switch (res.result_) {
                case server::RoomsCreationHandler::RoomCreatingResult::
                    ResultType::IncorrectPasswordFormat: {
                    std::cout << GetUser()
                              << " sent malformed password, no null "
                                 "termination. strange."
                              << std::endl;
                    SendEvent(events::CreateRoomAttempRespondEvent{
                        events::CreateRoomAttempRespondEvent::RespondType::
                            IncorrectPasswordFormat,
                        0});
                    break;
                }
                case server::RoomsCreationHandler::RoomCreatingResult::
                    ResultType::IncorrectNameFormat: {
                    std::cout << GetUser()
                              << " sent malformed name, no null "
                                 "termination. strange."
                              << std::endl;
                    SendEvent(events::CreateRoomAttempRespondEvent{
                        events::CreateRoomAttempRespondEvent::RespondType::
                            IncorrectNameFormat,
                        0});
                    break;
                }
                case server::RoomsCreationHandler::RoomCreatingResult::
                    ResultType::NameAlreadyExists: {
                    std::cout << GetUser()
                              << " tried creating room with taken name "
                              << roomData.name_ << std::endl;
                    SendEvent(events::CreateRoomAttempRespondEvent{
                        events::CreateRoomAttempRespondEvent::RespondType::
                            NameAlreadyExists,
                        0});
                    break;
                }
                default: {
                    std::cout << GetUser() << "'s attemp to join "
                              << roomData.name_
                              << " resulted in unknown respond" << std::endl;
                    SendEvent(events::CreateRoomAttempRespondEvent{
                        events::CreateRoomAttempRespondEvent::RespondType::
                            Unknown,
                        0});
                }
            }
        }
    } else
        UserHandler::_OnEvent(ev_data);
}

}  // namespace client
