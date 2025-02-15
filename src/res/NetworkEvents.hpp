#pragma once

namespace NetworkEventsNS {

    //including null termination character
    inline constexpr size_t ClientUsernameMaxLen = 16;
    //including null termination character
    inline constexpr size_t ClientPasswordMaxLen = 16;

    using EventsEnumType = unsigned char;

    enum class EventsTypesToClientE :EventsEnumType {
        ClientConnected, ClientDisconnected,
        LoginRequest, LoginResult,
    END_OF_ENUM};
    template<EventsTypesToClientE> struct EventTypeToClientS { static_assert(false, "this event is not specialized"); };
    template<> struct EventTypeToClientS<EventsTypesToClientE::ClientConnected> {
        char Username[ClientUsernameMaxLen];
    };
    template<> struct EventTypeToClientS<EventsTypesToClientE::ClientDisconnected> {
        char Username[ClientUsernameMaxLen];
    };
    template<> struct EventTypeToClientS<EventsTypesToClientE::LoginRequest> {};
    template<> struct EventTypeToClientS<EventsTypesToClientE::LoginResult> {
        enum class RespTypeE :unsigned char {
            Banned, WrongPassword, RegisteredAsNewUser, RegisteredAsExistingUser
        } RespType;
    };

    union EventTypeToClientU {
        EventTypeToClientS<EventsTypesToClientE::ClientConnected> ClientConnected;
        EventTypeToClientS<EventsTypesToClientE::ClientDisconnected> ClientDisconnected;
        EventTypeToClientS<EventsTypesToClientE::LoginRequest> LoginRequest;
        EventTypeToClientS<EventsTypesToClientE::LoginResult> LoginResult;
    };
#define EventsTypesToClientSwitchCaseMacro(eventEnum)\
    switch(eventEnum){\
    case NetworkEventsNS::EventsTypesToClientE::ClientConnected: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToClientS<NetworkEventsNS::EventsTypesToClientE::ClientConnected>); break; }\
    case NetworkEventsNS::EventsTypesToClientE::ClientDisconnected: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToClientS<NetworkEventsNS::EventsTypesToClientE::ClientDisconnected>); break; }\
    case NetworkEventsNS::EventsTypesToClientE::LoginRequest: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToClientS<NetworkEventsNS::EventsTypesToClientE::LoginRequest>); break; }\
    case NetworkEventsNS::EventsTypesToClientE::LoginResult: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToClientS<NetworkEventsNS::EventsTypesToClientE::LoginResult>); break; }\
    default: break;\
    }






    enum class EventsTypesToServerE :EventsEnumType {
        ClientConnected, ClientDisconnected,
        LoginRequestRespond,
    END_OF_ENUM};
    template<EventsTypesToServerE> struct EventTypeToServerS { static_assert(false, "this event is not specialized"); };
    template<> struct EventTypeToServerS<EventsTypesToServerE::ClientConnected> {};
    template<> struct EventTypeToServerS<EventsTypesToServerE::ClientDisconnected> {};
    template<> struct EventTypeToServerS<EventsTypesToServerE::LoginRequestRespond> {
        char Username[ClientUsernameMaxLen];
        char Password[ClientPasswordMaxLen];
    };
    
    union EventTypeToServerU {
        EventTypeToServerS<EventsTypesToServerE::ClientConnected> ClientConnected;
        EventTypeToServerS<EventsTypesToServerE::ClientDisconnected> ClientDisconnected;
        EventTypeToServerS<EventsTypesToServerE::LoginRequestRespond> LoginRequestRespond;
    };

#define EventsTypesToServerSwitchCaseMacro(eventEnum)\
    switch(eventEnum){\
    case NetworkEventsNS::EventsTypesToServerE::ClientConnected: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::ClientConnected>); break; }\
    case NetworkEventsNS::EventsTypesToServerE::ClientDisconnected: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::ClientDisconnected>); break; }\
    case NetworkEventsNS::EventsTypesToServerE::LoginRequestRespond: { SwitchCaseTempMacro(NetworkEventsNS::EventTypeToServerS<NetworkEventsNS::EventsTypesToServerE::LoginRequestRespond>); break; }\
    default: break;\
    }


}
