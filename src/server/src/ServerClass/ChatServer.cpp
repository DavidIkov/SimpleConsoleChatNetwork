#include"ChatServer.hpp"

#define OutputMacro ((OutputtingProcPtr==nullptr)?ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC():*OutputtingProcPtr)

ChatServerC::BasicClientS& ChatServerC::ClientFactory() {
    return *Clients.emplace(Clients.begin(), new ChatClientS(AsioContext.get()))->get();
}
void ChatServerC::OnEvent(BasicClientS& client, NetworkEventsNS::EventsTypesToServerE eventType, NetworkEventsNS::EventTypeToServerU const& eventData) {
    switch(eventType){
    case NetworkEventsNS::EventsTypesToServerE::ClientConnected: {
        OutputMacro << "login req sednded" << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        SendEvent(client, NetworkEventsNS::EventTypeToClientS<NetworkEventsNS::EventsTypesToClientE::LoginRequest>{});
        break;
    }
    case NetworkEventsNS::EventsTypesToServerE::ClientDisconnected: { break; }
    case NetworkEventsNS::EventsTypesToServerE::LoginRequestRespond: { 
        OutputMacro << "responded " << eventData.LoginRequestRespond.Username << ' ' << eventData.LoginRequestRespond.Password
            << ConsoleManagerNS::OutputNS::OutputtingProcessC::EndLine;
        break;
    }
    default: break;
    }
}