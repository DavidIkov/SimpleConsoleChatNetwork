#include"Server.hpp"

struct {
    ServerC* Server = nullptr;
} DataForCommands;

#include"ConsoleCommands.hpp"

int main(int argc, char** argv) {
    asio::io_context serverContext;
    asio::io_context::work idleWork(serverContext);
    std::thread serverThread([&] { serverContext.run(); });
    ServerC server(serverContext, 16120);
    DataForCommands.Server = &server;
    while (true) {
        std::string input;
        std::cin >> input;
        if (input == "exit") {
            idleWork.~work();
            server.Shutdown();
            break;
        }
    }
    serverThread.join();
}