#pragma once

namespace NetworkEventsNS {

    //including null termination character
    inline constexpr size_t ClientUsernameMaxLen = 16;
    //including null termination character
    inline constexpr size_t ClientPasswordMaxLen = 16;

    using EventsEnumType = unsigned char;

    enum class EventsTypesToClientE :EventsEnumType {
        UserConnected, UserDisconnected,
        LoginResult,
    END_OF_ENUM};
    template<EventsTypesToClientE> struct EventTypeToClientS { static_assert(false, "this event is not specialized"); };
    template<> struct EventTypeToClientS<EventsTypesToClientE::UserConnected> {
        char Username[ClientUsernameMaxLen];
    };
    template<> struct EventTypeToClientS<EventsTypesToClientE::UserDisconnected> {
        char Username[ClientUsernameMaxLen];
    };
    template<> struct EventTypeToClientS<EventsTypesToClientE::LoginResult> {
        enum class RespTypeE :unsigned char {
            Banned, WrongPassword, RegisteredAsNewUser, RegisteredAsExistingUser
        } RespType;
    };

    union EventTypeToClientU {
        EventTypeToClientS<EventsTypesToClientE::UserConnected> UserConnected;
        EventTypeToClientS<EventsTypesToClientE::UserDisconnected> UserDisconnected;
        EventTypeToClientS<EventsTypesToClientE::LoginResult> LoginResult;
    };
#define EventsTypesToClientSwitchCaseMacro(eventEnum)\
    switch(eventEnum){\
    case NetworkEventsNS::EventsTypesToClientE::UserConnected: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToClientS<NetworkEventsNS::EventsTypesToClientE::UserConnected>); break; }\
    case NetworkEventsNS::EventsTypesToClientE::UserDisconnected: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToClientS<NetworkEventsNS::EventsTypesToClientE::UserDisconnected>); break; }\
    case NetworkEventsNS::EventsTypesToClientE::LoginResult: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToClientS<NetworkEventsNS::EventsTypesToClientE::LoginResult>); break; }\
    default: break;\
    }






    enum class EventsTypesToServerE :EventsEnumType {
        ClientConnected, ClientDisconnected,
        LogInUser, UnlogFromUser, 
    END_OF_ENUM};
    template<EventsTypesToServerE> struct EventTypeToServerS { static_assert(false, "this event is not specialized"); };
    template<> struct EventTypeToServerS<EventsTypesToServerE::ClientConnected> {};
    template<> struct EventTypeToServerS<EventsTypesToServerE::ClientDisconnected> {};
    template<> struct EventTypeToServerS<EventsTypesToServerE::LogInUser> {
        char Username[ClientUsernameMaxLen];
        char Password[ClientPasswordMaxLen];
    };
    template<> struct EventTypeToServerS<EventsTypesToServerE::UnlogFromUser> {};
    
    union EventTypeToServerU {
        EventTypeToServerS<EventsTypesToServerE::ClientConnected> ClientConnected;
        EventTypeToServerS<EventsTypesToServerE::ClientDisconnected> ClientDisconnected;
        EventTypeToServerS<EventsTypesToServerE::LogInUser> LoginRequestRespond;
        EventTypeToServerS<EventsTypesToServerE::UnlogFromUser> UnlogFromUser;
    };

#define EventsTypesToServerSwitchCaseMacro(eventEnum)\
    switch(eventEnum){\
    case NetworkEventsNS::EventsTypesToServerE::ClientConnected: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::ClientConnected>); break; }\
    case NetworkEventsNS::EventsTypesToServerE::ClientDisconnected: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::ClientDisconnected>); break; }\
    case NetworkEventsNS::EventsTypesToServerE::LogInUser: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::LogInUser>); break; }\
    case NetworkEventsNS::EventsTypesToServerE::UnlogFromUser: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::UnlogFromUser>); break; }\
    default: break;\
    }


}
