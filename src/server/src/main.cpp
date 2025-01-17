

#include"Server.hpp"
#include<iostream>
int main(int argc, char** argv) {
    asio::io_context serverContext;
    asio::io_context::work idleWork(serverContext);
    std::thread serverThread([&] { serverContext.run(); });
    char buffer[100];
    ServerC server(serverContext, 16120, ServerC::ReadBufferS{ buffer, 100 });
    while (true) {
        std::string input;
        std::cin >> input;
        if (input == "stop") {
            idleWork.~work();
            server.Shutdown();
            break;
        }
    }
    serverThread.join();
}