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
    switch(eventType){
    case NetworkEventsNS::EventsTypesToServerE::LogInUser: {
        if (!Logged) {
            auto& respData = eventData.LoginRequestRespond;
            UserID = Server->GetUserIDByName(respData.Username);
            if (UserID == 0) {
                UserID = Server->RegisterNewUser(respData.Username, respData.Password);
                Logged = true;
                SendEvent(LogInResultEv{ LogInResultEv::RespTypeE::LoggedAsNewUser });
            }
            else {
                ChatServerC::LogInUserResultE logInRes = Server->LogInUser(UserID, respData.Password);
                LogInResultEv event;
                switch (logInRes) {
                case decltype(logInRes)::Banned: event.RespType = LogInResultEv::RespTypeE::Banned; break;
                case decltype(logInRes)::WrongPassword: event.RespType = LogInResultEv::RespTypeE::WrongPassword; break;
                case decltype(logInRes)::Logged: event.RespType = LogInResultEv::RespTypeE::LoggedAsExistingUser; Logged = true; break;
                default: event.RespType = LogInResultEv::RespTypeE::DeclinedWithUnknownReason; break;
                }
                SendEvent(event);
            }
        }
        else SendEvent(LogInResultEv{ LogInResultEv::RespTypeE::AlreadyLoggedIn });
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