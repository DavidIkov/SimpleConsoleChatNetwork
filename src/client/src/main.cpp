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

char ReadingBuffer[1024];
asio::io_context CurContext;
ClientC Client(CurContext, std::string_view(ReadingBuffer, 1024));

#include"ConsoleCommands.hpp"

int main(int argc, char** argv) {
    ConsoleManagerNS::Initialize();
    std::thread th1([&] {
        ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC proc;
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds((long long)((std::rand() & 0xf) / (float)0xf * 2123.f)));
            proc << "thread 1 output!" << proc.FlushOutput;
        }
        });
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::thread th2([&] {
        ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC proc;
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds((long long)((std::rand() & 0xf) / (float)0xf * 2123.f)));
            proc << "thread 2 output!" << proc.FlushOutput;
        }
        });
    th1.join(), th2.join();
    asio::error_code CurErrorCode;
    asio::io_context::work IdleWork(CurContext);
    std::thread ContextThread([&] {CurContext.run();});

    std::thread th = ConsoleCommandsNS::InitializeConsoleReadingThread();

    th.join();
    CurContext.stop();
    IdleWork.~work();
    ContextThread.join();
    return 0;
}

