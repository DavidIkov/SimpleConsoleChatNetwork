#include"ChatServer.hpp"

#define OutputMacro ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC()
#define ChatClientConv(client) dynamic_cast<ChatClientS&>(client)

using namespace NetworkEventsNS;

struct RegisteredUserS {
    char Username[ClientUsernameMaxLen];
    char Password[ClientPasswordMaxLen];
    bool Banned = false;
};
static std::vector<RegisteredUserS> RegisteredUsers;

ChatServerC::ChatServerC(asio::io_context& asioContext, asio::ip::port_type port) :EventsServerC(asioContext, port) {
    OutputMacro << "Server is running!" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
}
ChatServerC::BasicClientS& ChatServerC::ClientFactory() {
    std::lock_guard lg(ServerMutex);
    return *Clients.emplace(Clients.begin(), new ChatClientS(AsioContext.get()))->get();
}
void ChatServerC::OnAcceptConnectionError(OnAcceptConnectionErrorE err) {
    switch (err) {
    case OnAcceptConnectionErrorE::ServerClosedAcceptor: return;
    case OnAcceptConnectionErrorE::UnknownError:
        OutputMacro << "uknown error occured while trying to accept connection"
            << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        return;
    default:
        OutputMacro << "unhandled error occured while trying to accept connection"
            << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        return;
    }
}
void ChatServerC::OnEvent(BasicClientS& client, EventsTypesToServerE eventType, EventTypeToServerU const& eventData) {
    ChatClientS& chatClient = ChatClientConv(client);
    std::lock_guard lg(ServerMutex);
    switch(eventType){
    case EventsTypesToServerE::ClientConnected: {
        OutputMacro << "client joined from " << chatClient.Socket.remote_endpoint().address().to_string() << ':' <<
            std::to_string(chatClient.Socket.remote_endpoint().port()) << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        break;
    }
    case EventsTypesToServerE::ClientDisconnected: {
        if (chatClient.Registered)
            OnEvent(client, EventsTypesToServerE::LogOutFromUser, {});
        auto& data = eventData.ClientDisconnected;
        auto out = OutputMacro;
        out << "client ";
        if (data.Reason == decltype(eventData.ClientDisconnected)::DisconnectReasonE::ClientDisconnected)
            out << "disconnected";
        else if (data.Reason == decltype(eventData.ClientDisconnected)::DisconnectReasonE::ClientResetedConnection)
            out << "reseted connection";
        else if (data.Reason == decltype(eventData.ClientDisconnected)::DisconnectReasonE::ServerDisconnected)
            out << "got disconnected by server";
        else if (data.Reason == decltype(eventData.ClientDisconnected)::DisconnectReasonE::ServerShutdown)
            out << "got disconnected since server is shutting down";
        else if (data.Reason == decltype(eventData.ClientDisconnected)::DisconnectReasonE::UnknownError)
            out << "got disconnected becouse of unknown error";
        out<< ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        break;
    }
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