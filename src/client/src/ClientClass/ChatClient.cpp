#include"ChatClient.hpp"

#define OutputMacro ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC()

using namespace NetworkEventsNS;

ChatClientC::~ChatClientC() {
    if (IsChatClientDestructorLast) Mutex.lock();
}
ChatClientC::ChatClientC(asio::io_context& context) :EventsClientC(context) {
    IsEventsClientDestructorLast = false;
    OutputMacro << "Client is running!" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
}
void ChatClientC::OnEvent(EventsTypesToClientE eventType, EventTypeToClientU const& eventData) {
    //todo add login event
    switch (eventType) {
    case EventsTypesToClientE::ConnectedToServer: {
        OutputMacro << "Connected to server" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        break;
    }
    case EventsTypesToClientE::DisconnectedFromServer: {
        //todo handle reasons of disconnect
        LoggedInUserInServer = false;
        OutputMacro << "Disconnected from server" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        break;
    }
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

auto ChatClientC::LogIn(std::string username, std::string password) -> LogInResultE {
    std::lock_guard lg(Mutex);
    if (!_gIsConnected()) return LogInResultE::NotConnected;
    else if (_gIsLoggedInUserInServer()) return LogInResultE::AlreadyLogged;
    else if (username.size() > NetworkEventsNS::ClientUsernameMaxLen) return LogInResultE::UsernameTooLong;
    else if (password.size() > NetworkEventsNS::ClientPasswordMaxLen) return LogInResultE::PasswordTooLong;
    NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::LogInUser> evData;
    std::memcpy(&evData.Username, username.data(), username.size() + 1);
    std::memcpy(&evData.Password, password.data(), password.size() + 1);
    if (_SendEvent(evData) != SendEventResultE::NoErrors) return LogInResultE::FailedSendingEvent;
    return LogInResultE::NoErrors;
}
auto ChatClientC::LogOut()->LogOutResultE {
    std::lock_guard lg(Mutex);
    if (!_gIsConnected()) return LogOutResultE::NotConnected;
    else if (!_gIsLoggedInUserInServer()) return LogOutResultE::NotLoggedIn;
    LoggedInUserInServer = false;
    _SendEvent(NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::LogOutFromUser>{});
    return LogOutResultE::NoErrors;
}