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
#include"ConsoleCommands.hpp"

int main(int argc, char** argv) {
    ConsoleManagerNS::Initialize();

    asio::io_context CurContext;
    
    asio::io_context::work IdleWork(CurContext);
    std::thread ContextThread([&] {CurContext.run();});
    
    ClientC Client(CurContext);

    ConsoleCommandsNS::DataForCommands.Client = &Client;

    std::thread th = ConsoleCommandsNS::InitializeConsoleReadingThread();

    th.join();
    CurContext.stop();
    IdleWork.~work();
    ContextThread.join();
    ConsoleManagerNS::OutputNS::Terminate();
    return 0;
}

