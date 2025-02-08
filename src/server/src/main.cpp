#include"Server.hpp"
#include"ConsoleCommands.hpp"

int main(int argc, char** argv) {

    asio::io_context CurContext;
    ConsoleManagerNS::Initialize();
    asio::error_code CurErrorCode;
    asio::io_context::work IdleWork(CurContext);
    std::thread ContextThread([&] {CurContext.run();});
    ServerC server(CurContext, 16120);

    ConsoleCommandsNS::DataForCommands.Server = &server;
    std::thread th = ConsoleCommandsNS::InitializeConsoleReadingThread();
    th.join();
    server.Shutdown();
    CurContext.stop();
    IdleWork.~work();
    ContextThread.join();
    ConsoleManagerNS::OutputNS::Terminate();


    /*
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
    */
}