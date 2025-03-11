#include"ChatClientSlot.hpp"

void ChatClientSlotC::OnConnect() {
    BasicClientSlotC::OnConnect();

}
void ChatClientSlotC::OnDisconnect(DisconnectReasonE reason) {
    BasicClientSlotC::OnDisconnect(reason);
    if (Logged) { Server->LogOutFromUser(UserID); Logged = false; UserID = 0; }
}

using LogInResultEv = NetworkEventsNS::EventTypeToClientS<NetworkEventsNS::EventsTypesToClientE::LogInResult>;
void ChatClientSlotC::OnEvent(NetworkEventsNS::EventsTypesToServerE eventType, NetworkEventsNS::EventTypeToServerU const& eventData) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    switch(eventType){
    case NetworkEventsNS::EventsTypesToServerE::LogInUser: {
        if(!Logged) {
            auto& respData = eventData.LoginRequestRespond;
            uint64_t userID = Server->GetUserIDByName(respData.Username);
            if (userID == 0) {
                Server->RegisterNewUser(respData.Username, respData.Password);
                SendEvent(LogInResultEv{ LogInResultEv::RespTypeE::LoggedAsNewUser });
            }
            else {
                ChatServerC::LogInUserResultE logInRes = Server->LogInUser(userID, respData.Password);
                LogInResultEv event;
                switch (logInRes) {
                case decltype(logInRes)::Banned: event.RespType = LogInResultEv::RespTypeE::Banned; break;
                case decltype(logInRes)::WrongPassword: event.RespType = LogInResultEv::RespTypeE::WrongPassword; break;
                case decltype(logInRes)::Logged: event.RespType = LogInResultEv::RespTypeE::LoggedAsExistingUser; break;
                default: event.RespType = LogInResultEv::RespTypeE::DeclinedWithUnknownReason; break;
                }
                SendEvent(event);
            }
        }
        break;
    }
    case NetworkEventsNS::EventsTypesToServerE::LogOutFromUser: {
        if (Logged) {
            Logged = false;
            Server->LogOutFromUser(UserID);
        }
        break;
    }
    default: {
        break;
    }
    }
}