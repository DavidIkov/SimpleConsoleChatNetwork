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
    switch (eventType) {
    case EventsTypesToClientE::ConnectedToServer: {
        OutputMacro << "Connected to server" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        break;
    }
    case EventsTypesToClientE::DisconnectedFromServer: {
        //todo handle reasons of disconnect
        LoggedInUser = false;
        if (LoggingInUserEvent.Active) {
            LoggingInUserEvent.Stopped = true;
            CV.notify_all();
        }
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
        if (eventData.LogInResult.RespType == EventTypeToClientS<EventsTypesToClientE::LogInResult>::RespTypeE::LoggedAsExistingUser ||
            eventData.LogInResult.RespType == EventTypeToClientS<EventsTypesToClientE::LogInResult>::RespTypeE::LoggedAsNewUser)
            LoggedInUser = true;
        LoggingInUserEvent.ServerResponded = true;
        LoggingInUserEvent.ResponseType = eventData.LogInResult.RespType;
        CV.notify_all();
        break;
    }
    default: {
        OutputMacro << "unknown event on client" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        break;
    }
    }
}

auto ChatClientC::LogIn(std::string username, std::string password) -> LogInResultE {
    std::unique_lock ul(Mutex);
    if (!_gIsConnected()) return LogInResultE::NotConnected;
    else if (_gIsLoggedInUser()) return LogInResultE::AlreadyLogged;
    else if (username.size() > NetworkEventsNS::ClientUsernameMaxLen) return LogInResultE::UsernameTooLong;
    else if (password.size() > NetworkEventsNS::ClientPasswordMaxLen) return LogInResultE::PasswordTooLong;
    NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::LogInUser> evData;
    std::memcpy(&evData.Username, username.data(), username.size() + 1);
    std::memcpy(&evData.Password, password.data(), password.size() + 1);

    LoggingInUserEvent.Active = true;
    LoggingInUserEvent.ServerResponded = false, LoggingInUserEvent.Stopped = false;
    std::shared_ptr<bool> isDestructing = InstanceIsDestructing;
    std::thread waitingTh([&] {
        CV.wait(ul, [&]()->bool {return *isDestructing || LoggingInUserEvent.Stopped || LoggingInUserEvent.ServerResponded;});
        });
    if (_SendEvent(evData) != SendEventResultE::NoErrors) return LogInResultE::FailedSendingEvent;
    waitingTh.join();
    if (*isDestructing) { ul.release(); return LogInResultE::OperationAborted; }
    LoggingInUserEvent.Active = false;
    if (LoggingInUserEvent.Stopped) return LogInResultE::OperationAborted;
    switch(LoggingInUserEvent.ResponseType){
    case(decltype(LoggingInUserEvent.ResponseType)::Banned): return LogInResultE::Banned;
    case(decltype(LoggingInUserEvent.ResponseType)::LoggedAsExistingUser): return LogInResultE::LoggedAsExistingUser;
    case(decltype(LoggingInUserEvent.ResponseType)::LoggedAsNewUser): return LogInResultE::LoggedAsNewUser;
    case(decltype(LoggingInUserEvent.ResponseType)::WrongPassword): return LogInResultE::WrongPassword;
    default: return LogInResultE::UnknownRespond;
    }
}
auto ChatClientC::LogOut()->LogOutResultE {
    std::lock_guard lg(Mutex);
    if (!_gIsConnected()) return LogOutResultE::NotConnected;
    else if (!_gIsLoggedInUser()) return LogOutResultE::NotLoggedIn;
    LoggedInUser = false;
    _SendEvent(NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::LogOutFromUser>{});
    return LogOutResultE::NoErrors;
}