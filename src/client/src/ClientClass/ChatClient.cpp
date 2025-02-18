#include"ChatClient.hpp"

#define OutputMacro ((OutputtingProcPtr==nullptr)?ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC():*OutputtingProcPtr)

using namespace NetworkEventsNS;

void ChatClientC::OnEvent(EventsTypesToClientE eventType, EventTypeToClientU const& eventData) {
    std::lock_guard lg(EventMutex);
    switch (eventType) {
    case EventsTypesToClientE::UserConnected: {
        break;
    }
    case EventsTypesToClientE::UserDisconnected: {
        break;
    }
    case EventsTypesToClientE::LoginResult: {
        switch (eventData.LoginResult.RespType) {
        case EventTypeToClientS<EventsTypesToClientE::LoginResult>::RespTypeE::Banned: {
            OutputMacro << "login failed, user is banned" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            break;
        }
        case EventTypeToClientS<EventsTypesToClientE::LoginResult>::RespTypeE::WrongPassword: {
            OutputMacro << "login failed, wrong password" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            break;
        }
        case EventTypeToClientS<EventsTypesToClientE::LoginResult>::RespTypeE::RegisteredAsNewUser: {
            OutputMacro << "login succeed, registered as new user" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            RegisteredInServer = true;
            break;
        }
        case EventTypeToClientS<EventsTypesToClientE::LoginResult>::RespTypeE::RegisteredAsExistingUser: {
            OutputMacro << "login succeed, registered as existing user" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            RegisteredInServer = true;
            break;
        }
        default: {
            OutputMacro << "unknown server respond to a login attempt" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            break;
        }
        }
        break;
    }
    default: {
        OutputMacro << "unknown event on client" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        break;
    }
    }
}
void ChatClientC::OnConnect(){
    std::lock_guard lg(EventMutex);
    //todo not sure if there is need to do anything, maybe remove this event
}
void ChatClientC::OnDisconnect() {
    std::lock_guard lg(EventMutex);
    RegisteredInServer = false;
}
