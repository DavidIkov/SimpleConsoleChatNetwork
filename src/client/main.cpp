#include<iostream>
#include"AsioInclude.hpp"
#include"asio/ts/buffer.hpp"
#include"asio/ts/internet.hpp"
#include<thread>
#include<chrono>
#include<queue>
#include<condition_variable>
#include<functional>
#include"RemoveArrayPointer.hpp"
#include"Client.hpp"

asio::io_context CurContext;
ClientC Client(CurContext);

#include"ConsoleCommands.hpp"

/*
char BufferForText[100];
void StartReading(asio::ip::tcp::socket& socket) {
    socket.async_read_some(asio::buffer(BufferForText), [&](asio::error_code ec, size_t bytes) {
        if (ec) {
            if (ec == asio::error::eof) { std::cout << PutBeforeLastString << "Server disconnected, stopping reading" << std::endl; return; }
            else if (ec == asio::error::operation_aborted) { std::cout << PutBeforeLastString << "Client closed socket, stopping reading" << std::endl; return; }
            else if (ec == asio::error::connection_reset) { std::cout << PutBeforeLastString << "Server reseted connection, stopping reading" << std::endl; return; }
            else { std::cout << PutBeforeLastString << "Unhandled error occured while reading " << ec.value() << ' ' << ec.message() << std::endl; return; }
        }
        for (size_t i = 0;i < bytes;i++) std::cout << BufferForText[i]; std::cout << std::endl;
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
        StartReading(socket);
        });

}
*/

int main(int argc, char** argv) {

    using namespace std::chrono_literals;

    /*
    std::thread EventsThread([&] {
        while (true) {
            std::unique_lock ul(EventsNS::Mutex);
            EventsNS::CV.wait(ul, [&]()->bool {return EventsNS::_StopEventsThread || EventsNS::EventsQueue.size() > 0;});
            while (EventsNS::EventsQueue.size() > 0) {
                auto& ev = EventsNS::EventsQueue.front();
                auto out = ev.Fire();
                switch (ev.Type) {
                case EventsNS::TypesE::ConnectToServer:
                    if (out.ConnectToServer.ErrorCode) std::cout << PutBeforeLastString << "Failed connecting to server" << std::endl;
                    else std::cout << PutBeforeLastString << "Connected to server" << std::endl;
                    break;
                case EventsNS::TypesE::DisconnectFromServer:
                    std::cout << PutBeforeLastString << "Disconnected from server";
                    if (out.DisconnectFromServer.ErrorCode) std::cout << " with error " << out.DisconnectFromServer.ErrorCode.value() <<
                        " and message " << out.DisconnectFromServer.ErrorCode.message();
                    std::cout << std::endl;
                }
                EventsNS::EventsQueue.pop();
            }
            if (EventsNS::_StopEventsThread) return;
        }
        });
*/

    asio::error_code CurErrorCode;
    asio::io_context::work IdleWork(CurContext);
    std::thread ContextThread([&] {CurContext.run();});

    std::thread th = ConsoleCommandsNS::InitializeConsoleReadingThread();

    th.join();
    ContextThread.join();
    /*
startOfConnecting:
    {
        std::thread waitForEnd([&] {
            std::unique_lock ul(ConsoleCommandsNS::Mutex);
            ConsoleCommandsNS::CV_End.wait(ul, [&]()->bool {return !ConsoleCommandsNS::WaitingForInput || ConsoleCommandsNS::_StopReading;});
            if (ConsoleCommandsNS::_StopReading) return;
            
            std::string ipToConnect, port;
            size_t fs=ConsoleCommandsNS::CommandBuffer.find_first_of(' ')
            });
        {
            std::lock_guard lg(ConsoleCommandsNS::Mutex);
            ConsoleCommandsNS::WaitingForInput = true;
            ConsoleCommandsNS::CV_Start.notify_all();
        }
        waitForEnd.join();
    }
    std::string ipToConnect; std::cout << PutBeforeLastString << "ip to connect: "; std::cin >> ipToConnect;
    std::string portStr; std::cout << PutBeforeLastString << "port to connect: "; std::cin >> portStr;
    {
        std::lock_guard lg(EventsNS::Mutex);
        EventsNS::EventsQueue.emplace(EventsNS::EventDataS{ EventsNS::TypesE::ConnectToServer, EventsNS::ConnectToServerS{ ClientSocket,asio::ip::tcp::endpoint(asio::ip::make_address(ipToConnect, CurErrorCode), std::stoi(portStr)) } });
        EventsNS::CV.notify_all();
    }
    std::this_thread::sleep_for(1000ms);
    if(!CurErrorCode && ClientSocket.is_open()){
        std::cout << PutBeforeLastString << "Client connected!" << std::endl;
        std::string username; std::cout << "your username: "; std::cin >> username;
        ClientSocket.write_some(asio::buffer(username));
        std::string msg;
        StartReading(ClientSocket);
        while (true) {
            std::cin >> msg;
            if (msg == "disconnect") {
                std::lock_guard lg(EventsNS::Mutex);
                EventsNS::EventsQueue.emplace(EventsNS::EventDataS{ EventsNS::TypesE::DisconnectFromServer, EventsNS::DisconnectFromServerS{ CurContext, ClientSocket } });
                EventsNS::CV.notify_all();
                goto startOfConnecting;
            }
            ClientSocket.write_some(asio::buffer(msg));
        }
    }
    else {
        std::cout << "client failed to connect :(\n";
        goto startOfConnecting;
    }
    {
        std::lock_guard lg(EventsNS::Mutex);
        EventsNS::EventsQueue.emplace(EventsNS::EventDataS{ EventsNS::TypesE::DisconnectFromServer, EventsNS::DisconnectFromServerS{ CurContext, ClientSocket } });
        EventsNS::CV.notify_all();
    }
    IdleWork.~work();
    ContextThread.join();
    EventsNS::StopEventsThread();
    CommandsThread.join();
    */
    return 0;
}

