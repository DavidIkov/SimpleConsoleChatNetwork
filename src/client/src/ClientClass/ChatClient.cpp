#include"ChatClient.hpp"

#define OutputMacro ((OutputtingProcPtr==nullptr)?ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC():*OutputtingProcPtr)

void ChatClientC::OnEvent(NetworkEventsNS::EventsTypesToClientE eventType, NetworkEventsNS::EventTypeToClientU const& eventData) {
    switch (eventType) {
    case NetworkEventsNS::EventsTypesToClientE::ClientConnected: {
        break;
    }
    case NetworkEventsNS::EventsTypesToClientE::ClientDisconnected: {
        break;
    }
    case NetworkEventsNS::EventsTypesToClientE::LoginRequest: {
        WaitingForLogin = true;
    }
    case NetworkEventsNS::EventsTypesToClientE::LoginResult: {
        if (eventData.LoginResult.Pass) {
            RegisteredInServer = true;
            WaitingForLogin = false;
        }
        else {
            RegisteredInServer = false;
            WaitingForLogin = true;
        }
    }
    default: break;
    }
}
void ChatClientC::OnConnect(){
    //todo not sure if there is need to do anything, maybe remove this event
}
void ChatClientC::OnDisconnect() {
    WaitingForLogin = false;
    RegisteredInServer = false;
}
