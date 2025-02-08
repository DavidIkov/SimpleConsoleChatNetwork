#pragma once

#define CLIENT_NAME_MAX_LEN 16
#define CLIENT_PASSWORD_MAX_LEN 16

namespace NetworkEventsNS {

    using EventsEnumType = unsigned char;

    enum class EventsTypesToClientE :EventsEnumType {
        ClientConnected, ClientDisconnected,
        LoginRequest, LoginResult,
    };
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



    enum class EventsTypesToServerE :EventsEnumType {
        ClientConnected, ClientDisconnected,
        LoginRequestRespond,
    };
    template<EventsTypesToServerE> struct EventTypeToServerS { static_assert(false, "this event is not specialized"); };
    template<> struct EventTypeToServerS<EventsTypesToServerE::ClientConnected> {};
    template<> struct EventTypeToServerS<EventsTypesToServerE::ClientDisconnected> {};
    template<> struct EventTypeToServerS<EventsTypesToServerE::LoginRequestRespond> {
        char Username[CLIENT_NAME_MAX_LEN];
        char Password[CLIENT_PASSWORD_MAX_LEN];
    };
}
