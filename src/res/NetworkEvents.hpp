#pragma once

namespace NetworkEventsNS {

    //including null termination character
    inline constexpr size_t ClientUsernameMaxLen = 15;
    //including null termination character
    inline constexpr size_t ClientPasswordMaxLen = 15;

    using EventsEnumType = unsigned char;

    enum class EventsTypesToClientE :EventsEnumType {
        ConnectedToServer, DisconnectedFromServer,
        UserConnected, UserDisconnected,
        LogInResult,
    END_OF_ENUM};
    template<EventsTypesToClientE> struct EventTypeToClientS { static_assert(false, "this event is not specialized"); };
    template<> struct EventTypeToClientS<EventsTypesToClientE::ConnectedToServer> {};
    template<> struct EventTypeToClientS<EventsTypesToClientE::DisconnectedFromServer> {
        enum class DisconnectReasonE :unsigned char {
            ServerDisconnected, ClientClosedSocket, ClientDisconnected, ServerResetedConnection, Unknown
        } Reason;
    };
    template<> struct EventTypeToClientS<EventsTypesToClientE::UserConnected> {
        char Username[ClientUsernameMaxLen + 1];
    };
    template<> struct EventTypeToClientS<EventsTypesToClientE::UserDisconnected> {
        char Username[ClientUsernameMaxLen + 1];
    };
    template<> struct EventTypeToClientS<EventsTypesToClientE::LogInResult> {
        enum class RespTypeE :unsigned char {
            Banned, WrongPassword, LoggedAsNewUser, LoggedAsExistingUser
        } RespType;
    };

    union EventTypeToClientU {
        EventTypeToClientS<EventsTypesToClientE::ConnectedToServer> ConnectedToServer;
        EventTypeToClientS<EventsTypesToClientE::DisconnectedFromServer> DisconnectedFromServer;
        EventTypeToClientS<EventsTypesToClientE::UserConnected> UserConnected;
        EventTypeToClientS<EventsTypesToClientE::UserDisconnected> UserDisconnected;
        EventTypeToClientS<EventsTypesToClientE::LogInResult> LogInResult;
    };
#define EventsTypesToClientSwitchCaseMacro(eventEnum)\
    switch(eventEnum){\
    case NetworkEventsNS::EventsTypesToClientE::ConnectedToServer: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToClientS<NetworkEventsNS::EventsTypesToClientE::ConnectedToServer>); break; }\
    case NetworkEventsNS::EventsTypesToClientE::DisconnectedFromServer: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToClientS<NetworkEventsNS::EventsTypesToClientE::DisconnectedFromServer>); break; }\
    case NetworkEventsNS::EventsTypesToClientE::UserConnected: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToClientS<NetworkEventsNS::EventsTypesToClientE::UserConnected>); break; }\
    case NetworkEventsNS::EventsTypesToClientE::UserDisconnected: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToClientS<NetworkEventsNS::EventsTypesToClientE::UserDisconnected>); break; }\
    case NetworkEventsNS::EventsTypesToClientE::LogInResult: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToClientS<NetworkEventsNS::EventsTypesToClientE::LogInResult>); break; }\
    default: break;\
    }






    enum class EventsTypesToServerE :EventsEnumType {
        ClientConnected, ClientDisconnected,
        LogInUser, LogOutFromUser, 
    END_OF_ENUM};
    template<EventsTypesToServerE> struct EventTypeToServerS { static_assert(false, "this event is not specialized"); };
    template<> struct EventTypeToServerS<EventsTypesToServerE::ClientConnected> {};
    template<> struct EventTypeToServerS<EventsTypesToServerE::ClientDisconnected> {
        enum class DisconnectReasonE :unsigned char {
            ServerShutdown, ServerDisconnected, ClientDisconnected, ClientResetedConnection, UnknownError,
        } Reason;
    };
    template<> struct EventTypeToServerS<EventsTypesToServerE::LogInUser> {
        char Username[ClientUsernameMaxLen];
        char Password[ClientPasswordMaxLen];
    };
    template<> struct EventTypeToServerS<EventsTypesToServerE::LogOutFromUser> {};
    
    union EventTypeToServerU {
        EventTypeToServerS<EventsTypesToServerE::ClientConnected> ClientConnected;
        EventTypeToServerS<EventsTypesToServerE::ClientDisconnected> ClientDisconnected;
        EventTypeToServerS<EventsTypesToServerE::LogInUser> LoginRequestRespond;
        EventTypeToServerS<EventsTypesToServerE::LogOutFromUser> LogOutFromUser;
    };

#define EventsTypesToServerSwitchCaseMacro(eventEnum)\
    switch(eventEnum){\
    case NetworkEventsNS::EventsTypesToServerE::ClientConnected: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::ClientConnected>); break; }\
    case NetworkEventsNS::EventsTypesToServerE::ClientDisconnected: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::ClientDisconnected>); break; }\
    case NetworkEventsNS::EventsTypesToServerE::LogInUser: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::LogInUser>); break; }\
    case NetworkEventsNS::EventsTypesToServerE::LogOutFromUser: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::LogOutFromUser>); break; }\
    default: break;\
    }


}
