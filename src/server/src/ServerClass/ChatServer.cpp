#include"ChatServer.hpp"
#include"ClientSlotClass/ChatClientSlot.hpp"

#define OutputMacro ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC()

using namespace NetworkEventsNS;

struct {
    struct DataS {
        char Username[ClientUsernameMaxLen];
        char Password[ClientPasswordMaxLen];
        bool Banned = false;
    };
    std::vector<DataS> Data;
    std::recursive_mutex Mutex;
} static RegisteredUsersDataBase;

ChatServerC::ChatServerC(asio::io_context& asioContext, asio::ip::port_type port) :EventsServerC(asioContext, port) {
    IsEventsDestructorLast = false;
    OutputMacro << "Server is running!" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
}
BasicClientSlotC& ChatServerC::ClientFactory(asio::io_context& context) {
    return *Clients.emplace(Clients.begin(), new ChatClientSlotC(context, *this))->get();
}
void ChatServerC::OnConnect(BasicClientSlotC& client) {
    BasicServerC::OnConnect(client);
    asio::ip::tcp::endpoint EP = client.gConnectedEndpoint();
    OutputMacro << "client joined from " << EP.address().to_string() << ':' <<
            std::to_string(EP.port()) << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
}
void ChatServerC::OnDisconnect(BasicClientSlotC& client, DisconnectReasonE reason) {
    BasicServerC::OnDisconnect(client, reason);
    ChatClientSlotC& chatClient = dynamic_cast<ChatClientSlotC&>(client);
    auto out = OutputMacro;
    out << "client ";
    if (reason == DisconnectReasonE::ClientDisconnected)
        out << "disconnected";
    else if (reason == DisconnectReasonE::ClientResetedConnection)
        out << "reseted connection";
    else if (reason == DisconnectReasonE::ServerDisconnected)
        out << "got disconnected by server";
    else if (reason == DisconnectReasonE::ServerShutdown)
        out << "got disconnected since server is shutting down";
    else if (reason == DisconnectReasonE::UnknownError)
        out << "got disconnected becouse of unknown error";
    out<< ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
}
void ChatServerC::OnAcceptConnectionFailure() {
    BasicServerC::OnAcceptConnectionFailure();
    OutputMacro << "uknown error occured while trying to accept connection"
        << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
}
uint64_t ChatServerC::GetUserIDByName(std::string const& username) {
    std::lock_guard LG(RegisteredUsersDataBase.Mutex);
    for (uint64_t i = 0;i < RegisteredUsersDataBase.Data.size();i++)
        if (RegisteredUsersDataBase.Data[i].Username == username) return i + 1;
    return 0;
}
std::string ChatServerC::GetUserNameByID(uint64_t ID) {
    std::lock_guard LG(RegisteredUsersDataBase.Mutex);
    if (CheckIfUserIDIsValid(ID)) return RegisteredUsersDataBase.Data[ID - 1].Username;
    return {};
}
bool ChatServerC::CheckIfUserIDIsValid(uint64_t ID){
    std::lock_guard LG(RegisteredUsersDataBase.Mutex);
    if (ID - 1 < RegisteredUsersDataBase.Data.size()) return true;
    return false;
}
auto ChatServerC::LogInUser(uint64_t userID, std::string const& password) -> LogInUserResultE {
    std::lock_guard LG(RegisteredUsersDataBase.Mutex);
    if (!CheckIfUserIDIsValid(userID)) return LogInUserResultE::InvalidID;
    auto& user = RegisteredUsersDataBase.Data[userID - 1];
    if (user.Password == password) {
        if (user.Banned) {
            OutputMacro << "logged in user \"" << user.Username << "\" but this user is banned, rejected"
                << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            return LogInUserResultE::Banned;
        }
        else {
            OutputMacro << "logged in user \"" << user.Username << "\""
                << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            return LogInUserResultE::Logged;
        }
    }
    else {
        OutputMacro << "failed to log in user \"" << user.Username << "\" becouse of wrong password"
            << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        return LogInUserResultE::WrongPassword;
    }
}
uint64_t ChatServerC::RegisterNewUser(std::string const& username, std::string const& password) {
    std::lock_guard LG(RegisteredUsersDataBase.Mutex);
    if (GetUserIDByName(username) != 0) return 0;
    auto& user = RegisteredUsersDataBase.Data.emplace_back();
    strncpy_s(user.Username, ClientUsernameMaxLen, username.data(), username.size() + 1);
    strncpy_s(user.Password, ClientPasswordMaxLen, password.data(), password.size() + 1);
    OutputMacro << "registered new user \"" << RegisteredUsersDataBase.Data.rbegin()->Username << "\""
        << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
    return RegisteredUsersDataBase.Data.size();
}
void ChatServerC::LogOutFromUser(uint64_t userID) {
    std::lock_guard LG(RegisteredUsersDataBase.Mutex);
    OutputMacro << "logged out from user \"" << RegisteredUsersDataBase.Data[userID - 1].Username << "\""
        << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
    //todo online/offline system for users
}

    /*
    case EventsTypesToServerE::LogInUser: {
        if(!chatClient.Registered) {
            auto& respData = eventData.LoginRequestRespond;
            bool foundUser = false;
            for (size_t userInd = 0; userInd < RegisteredUsers.size(); userInd++) {
                auto& user = RegisteredUsers[userInd];
                if (std::strcmp(user.Username, respData.Username) == 0) {
                    foundUser = true;
                    if (user.Banned) {
                        SendEvent(client, EventTypeToClientS<EventsTypesToClientE::LogInResult>{
                            EventTypeToClientS<EventsTypesToClientE::LogInResult>::RespTypeE::Banned });
                        OutputMacro << "client tried to register as user \"" << user.Username << "\" which is banned"
                            << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                    }
                    else if (std::strcmp(user.Password, respData.Password) == 0) {
                        chatClient.Registered = true;
                        chatClient.UserInd = userInd;
                        SendEvent(client, EventTypeToClientS<EventsTypesToClientE::LogInResult>{
                            EventTypeToClientS<EventsTypesToClientE::LogInResult>::RespTypeE::LoggedAsExistingUser });
                        OutputMacro << "client registered as existing user \"" << user.Username << "\""
                            << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                    }
                    else {
                        SendEvent(client, EventTypeToClientS<EventsTypesToClientE::LogInResult>{
                            EventTypeToClientS<EventsTypesToClientE::LogInResult>::RespTypeE::WrongPassword });
                        OutputMacro << "client tried to register as user \"" << user.Username << "\" but got password wrong"
                            << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                    }
                    break;
                }
            }
            if (!foundUser) {
                chatClient.Registered = true;
                chatClient.UserInd = RegisteredUsers.size();
                RegisteredUsers.emplace_back();
                std::memcpy(&RegisteredUsers.rbegin()->Username, respData.Username, ClientUsernameMaxLen);
                std::memcpy(&RegisteredUsers.rbegin()->Password, respData.Password, ClientPasswordMaxLen);
                OutputMacro << "client registered as new user \"" << respData.Username << "\""
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                SendEvent(client, EventTypeToClientS<EventsTypesToClientE::LogInResult>{
                    EventTypeToClientS<EventsTypesToClientE::LogInResult>::RespTypeE::LoggedAsNewUser});
            }
        }
        break;
    }
    case EventsTypesToServerE::LogOutFromUser: {
        if (chatClient.Registered) {
            chatClient.Registered = false;
            OutputMacro << "user " << RegisteredUsers[chatClient.UserInd].Username << " unlogged"
                << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        }
        break;
    }
    default: {
        OutputMacro << "unkown event on server" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        break;
    }
    }
}
    */