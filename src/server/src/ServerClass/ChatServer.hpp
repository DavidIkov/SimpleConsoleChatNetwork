#pragma once
#include"EventsServer.hpp"

class ChatServerC : public EventsServerC {
public:
    ChatServerC(asio::io_context& asioContext, asio::ip::port_type port);
    using EventsServerC::EventsServerC;
    virtual ~ChatServerC() override = default;
protected:
    struct ChatClientS :virtual public BasicClientS {
        bool Registered = false;//means that login was correct and so server accepted it
        size_t UserInd = 0;//index in vector of registered users
        using BasicClientS::BasicClientS;
        virtual ~ChatClientS() = default;
    };
    virtual BasicClientS& ClientFactory() override;
private:
    virtual void OnAcceptConnectionError(OnAcceptConnectionErrorE) override final;
    virtual void OnEvent(BasicClientS& client, NetworkEventsNS::EventsTypesToServerE eventType, NetworkEventsNS::EventTypeToServerU const& eventData) override final;
};