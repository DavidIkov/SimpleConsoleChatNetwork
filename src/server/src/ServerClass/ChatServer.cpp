#include"ChatServer.hpp"

#define OutputMacro ((OutputtingProcPtr==nullptr)?ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC():*OutputtingProcPtr)
#define ChatClientConv(client) dynamic_cast<ChatClientS&>(client)

using namespace NetworkEventsNS;

struct RegisteredUserS {
    char Username[ClientUsernameMaxLen];
    char Password[ClientPasswordMaxLen];
    bool Banned = false;
};
static std::vector<RegisteredUserS> RegisteredUsers;

ChatServerC::BasicClientS& ChatServerC::ClientFactory() {
    return *Clients.emplace(Clients.begin(), new ChatClientS(AsioContext.get()))->get();
}
#include<iostream>
void ChatServerC::OnEvent(BasicClientS& client, EventsTypesToServerE eventType, EventTypeToServerU const& eventData) {
    ChatClientS& chatClient = ChatClientConv(client);
    std::lock_guard lg(chatClient.EventMutex);
    switch(eventType){
    case EventsTypesToServerE::ClientConnected: {
        OutputMacro << "client joined from " << chatClient.Socket.remote_endpoint().address().to_string()
            << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        break;
    }
    case EventsTypesToServerE::ClientDisconnected: {
        if (chatClient.Registered)
            OnEvent(client, EventsTypesToServerE::UnlogFromUser, {});
        OutputMacro << "client disconnected" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
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
                        SendEvent(client, EventTypeToClientS<EventsTypesToClientE::LoginResult>{
                            EventTypeToClientS<EventsTypesToClientE::LoginResult>::RespTypeE::Banned });
                        OutputMacro << "client tried to register as user \"" << user.Username << "\" which is banned"
                            << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                    }
                    else if (std::strcmp(user.Password, respData.Password) == 0) {
                        chatClient.Registered = true;
                        chatClient.UserInd = userInd;
                        SendEvent(client, EventTypeToClientS<EventsTypesToClientE::LoginResult>{
                            EventTypeToClientS<EventsTypesToClientE::LoginResult>::RespTypeE::RegisteredAsExistingUser });
                        OutputMacro << "client registered as existing user \"" << user.Username << "\""
                            << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
                    }
                    else {
                        SendEvent(client, EventTypeToClientS<EventsTypesToClientE::LoginResult>{
                            EventTypeToClientS<EventsTypesToClientE::LoginResult>::RespTypeE::WrongPassword });
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
                SendEvent(client, EventTypeToClientS<EventsTypesToClientE::LoginResult>{
                    EventTypeToClientS<EventsTypesToClientE::LoginResult>::RespTypeE::RegisteredAsNewUser});
                OutputMacro << "client registered as new user \"" << respData.Username << "\""
                    << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
            }
        }
        break;
    }
    case EventsTypesToServerE::UnlogFromUser: {
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