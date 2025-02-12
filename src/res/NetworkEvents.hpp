#pragma once

#define CLIENT_NAME_MAX_LEN 16
#define CLIENT_PASSWORD_MAX_LEN 16

namespace NetworkEventsNS {

    using EventsEnumType = unsigned char;

    enum class EventsTypesToClientE :EventsEnumType {
        ClientConnected, ClientDisconnected,
        LoginRequest, LoginResult,
    END_OF_ENUM};
    template<EventsTypesToClientE> struct EventTypeToClientS { static_assert(false, "this event is not specialized"); };
    template<> struct EventTypeToClientS<EventsTypesToClientE::ClientConnected> {
        char Username[CLIENT_NAME_MAX_LEN];
    };
    template<> struct EventTypeToClientS<EventsTypesToClientE::ClientDisconnected> {
        char Username[CLIENT_NAME_MAX_LEN];
    };
    template<> struct EventTypeToClientS<EventsTypesToClientE::LoginRequest> {};
    template<> struct EventTypeToClientS<EventsTypesToClientE::LoginResult> {
        bool Pass;
    };

    union EventTypeToClientU {
        EventTypeToClientS<EventsTypesToClientE::ClientConnected> ClientConnected;
        EventTypeToClientS<EventsTypesToClientE::ClientDisconnected> ClientDisconnected;
        EventTypeToClientS<EventsTypesToClientE::LoginRequest> LoginRequest;
        EventTypeToClientS<EventsTypesToClientE::LoginResult> LoginResult;
    };
#define EventsTypesToClientSwitchCaseMacro(eventEnum)\
    switch(eventEnum){\
    case NetworkEventsNS::EventsTypesToClientE::ClientConnected: { SwitchCaseTempMacro(NetworkEventsNS::EventsTypesToClientE::ClientConnected); break; }\
    case NetworkEventsNS::EventsTypesToClientE::ClientDisconnected: { SwitchCaseTempMacro(NetworkEventsNS::EventsTypesToClientE::ClientDisconnected); break; }\
    case NetworkEventsNS::EventsTypesToClientE::LoginRequest: { SwitchCaseTempMacro(NetworkEventsNS::EventsTypesToClientE::LoginRequest); break; }\
    case NetworkEventsNS::EventsTypesToClientE::LoginResult: { SwitchCaseTempMacro(NetworkEventsNS::EventsTypesToClientE::LoginResult); break; }\
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
        char Username[CLIENT_NAME_MAX_LEN];
        char Password[CLIENT_PASSWORD_MAX_LEN];
    };
    
    union EventTypeToServerU {
        EventTypeToServerS<EventsTypesToServerE::ClientConnected> ClientConnected;
        EventTypeToServerS<EventsTypesToServerE::ClientDisconnected> ClientDisconnected;
        EventTypeToServerS<EventsTypesToServerE::LoginRequestRespond> LoginRequestRespond;
    };

#define EventsTypesToServerSwitchCaseMacro(eventEnum)\
    switch(eventEnum){\
    case NetworkEventsNS::EventsTypesToServerE::ClientConnected: { SwitchCaseTempMacro(NetworkEventsNS::EventsTypesToServerE::ClientConnected); break; }\
    case NetworkEventsNS::EventsTypesToServerE::ClientDisconnected: { SwitchCaseTempMacro(NetworkEventsNS::EventsTypesToServerE::ClientDisconnected); break; }\
    case NetworkEventsNS::EventsTypesToServerE::LoginRequestRespond: { SwitchCaseTempMacro(NetworkEventsNS::EventsTypesToServerE::LoginRequestRespond); break; }\
    default: break;\
    }


}
