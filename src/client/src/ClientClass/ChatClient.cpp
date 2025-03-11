#include"ChatClient.hpp"

#define OutputMacro ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC()

using namespace NetworkEventsNS;

ChatClientC::~ChatClientC() {
    if (IsChatClientDestructorLast) ThreadSafety.LockThread();
}
ChatClientC::ChatClientC(asio::io_context& context) :EventsClientC(context) {
    IsEventsClientDestructorLast = false;
    OutputMacro << "Client is running!" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
}
void ChatClientC::OnConnect() {
    BasicClientC::OnConnect();
    OutputMacro << "Connected to server" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
}
void ChatClientC::OnDisconnect(DisconnectReasonE reason) {
    BasicClientC::OnDisconnect(reason);
    //todo handle reasons of disconnect
    LoggedInUser = false;
    if (LoggingInUserEvent.Active) {
        LoggingInUserEvent.Stopped = true;
        ThreadSafety.gCV().notify_all();
    }
    OutputMacro << "Disconnected from server" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
}
#include<iostream>
void ChatClientC::OnEvent(EventsTypesToClientE eventType, EventTypeToClientU const& eventData) {
    std::cout << "\n\nOnEvent\n\n" << std::flush;
    EventsClientC::OnEvent(eventType, eventData);
    switch (eventType) {
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
        if (LoggingInUserEvent.Active) {
            LoggingInUserEvent.ServerResponded = true;
            LoggingInUserEvent.ResponseType = eventData.LogInResult.RespType;
            ThreadSafety.gCV().notify_all();
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
    ThreadLockC TL(this);
    if (!TL) return LogInResultE::Canceled;
    if (!gIsConnected()) return LogInResultE::NotConnected;
    else if (gIsLoggedInUser()) return LogInResultE::AlreadyLogged;
    else if (username.size() > NetworkEventsNS::ClientUsernameMaxLen) return LogInResultE::UsernameTooLong;
    else if (password.size() > NetworkEventsNS::ClientPasswordMaxLen) return LogInResultE::PasswordTooLong;
    NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::LogInUser> evData;
    std::memcpy(&evData.Username, username.data(), username.size() + 1);
    std::memcpy(&evData.Password, password.data(), password.size() + 1);

    LoggingInUserEvent.Active = true;
    LoggingInUserEvent.ServerResponded = false, LoggingInUserEvent.Stopped = false;
    if (SendEvent(evData) != SendEventResultE::NoErrors) return LogInResultE::FailedSendingEvent;
    /*
    ThreadSafety.LockDepth--;
    ThreadSafety.Data->Mutex.unlock();
    std::this_thread::sleep_for(std::chrono::seconds(6));
    std::cout << "\nfinish wait" << std::endl;
    ThreadSafety.Data->Mutex.lock();
    ThreadSafety.LockDepth++;
    */
    std::this_thread::sleep_for(std::chrono::seconds(10));
    std::cout << "\nLOGIN WAIT START" << std::endl;
    TL.Wait([&]()->bool {return !TL || LoggingInUserEvent.Stopped || LoggingInUserEvent.ServerResponded;});
    std::cout << "\nLOGIN WAIT END" << std::endl;
    if (!TL) return LogInResultE::Canceled;
    LoggingInUserEvent.Active = false;
    if (LoggingInUserEvent.Stopped) return LogInResultE::Canceled;
    switch(LoggingInUserEvent.ResponseType){
    case(decltype(LoggingInUserEvent.ResponseType)::Banned): return LogInResultE::Banned;
    case(decltype(LoggingInUserEvent.ResponseType)::LoggedAsExistingUser): return LogInResultE::LoggedAsExistingUser;
    case(decltype(LoggingInUserEvent.ResponseType)::LoggedAsNewUser): return LogInResultE::LoggedAsNewUser;
    case(decltype(LoggingInUserEvent.ResponseType)::WrongPassword): return LogInResultE::WrongPassword;
    default: return LogInResultE::UnknownRespond;
    }
}
auto ChatClientC::LogOut()->LogOutResultE {
    ThreadLockC TL(this);
    if (!TL) return LogOutResultE::Canceled;
    if (!gIsConnected()) return LogOutResultE::NotConnected;
    else if (!gIsLoggedInUser()) return LogOutResultE::NotLoggedIn;
    LoggedInUser = false;
    SendEventResultE res = SendEvent(NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::LogOutFromUser>{});
    if (res == SendEventResultE::Canceled) return LogOutResultE::Canceled;
    else if (res == SendEventResultE::UnknownError) return LogOutResultE::UnknownError;
    return LogOutResultE::NoErrors;
}