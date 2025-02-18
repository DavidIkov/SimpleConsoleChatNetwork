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
    case EventsTypesToClientE::LogInResult: {
        switch (eventData.LogInResult.RespType) {
        case EventTypeToClientS<EventsTypesToClientE::LogInResult>::RespTypeE::Banned: {
            OutputMacro << "login failed, user is banned" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            break;
        }
        case EventTypeToClientS<EventsTypesToClientE::LogInResult>::RespTypeE::WrongPassword: {
            OutputMacro << "login failed, wrong password" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            break;
        }
        case EventTypeToClientS<EventsTypesToClientE::LogInResult>::RespTypeE::LoggedAsNewUser: {
            OutputMacro << "login succeed, registered as new user" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            LoggedInUserInServer = true;
            break;
        }
        case EventTypeToClientS<EventsTypesToClientE::LogInResult>::RespTypeE::LoggedAsExistingUser: {
            OutputMacro << "login succeed, registered as existing user" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            LoggedInUserInServer = true;
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
    LoggedInUserInServer = false;
}

auto ChatClientC::LogIn(std::string username, std::string password) -> LogInResultE {
    if (!gIsConnected()) return LogInResultE::NotConnectedToServer;
    else if (gIsLoggedInUserInServer()) return LogInResultE::AlreadyLogged;
    else if (username.size() > NetworkEventsNS::ClientUsernameMaxLen) return LogInResultE::UsernameTooLong;
    else if (password.size() > NetworkEventsNS::ClientPasswordMaxLen) return LogInResultE::PasswordTooLong;
    NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::LogInUser> evData;
    std::memcpy(&evData.Username, username.data(), username.size() + 1);
    std::memcpy(&evData.Password, password.data(), password.size() + 1);
    SendEvent(evData);
    return LogInResultE::NoErrors;
}
auto ChatClientC::LogOut()->LogOutResultE {
    if (!gIsConnected()) return LogOutResultE::NotConnectedToServer;
    else if (!gIsLoggedInUserInServer()) return LogOutResultE::NotLoggedIn;
    LoggedInUserInServer = false;
    SendEvent(NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::LogOutFromUser>{});
    return LogOutResultE::NoErrors;
}