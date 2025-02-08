#pragma once
#include"AsioInclude.hpp"
#include"RemoveArrayPointer.hpp"
#include"ConsoleManager.hpp"
#include"Server.hpp"
#include<string>
#include<mutex>
namespace ConsoleCommandsNS {

    struct {
        ServerC* Server = nullptr;
    } DataForCommands;
    
    std::mutex Mutex;
    std::string CommandBuffer;
    bool StopReading = false;
    namespace CommandsNS {
        std::pair<std::string_view, void(*)(ConsoleManagerNS::OutputNS::OutputtingProcessC&)> Commands[] = {
            {"list clients",[](ConsoleManagerNS::OutputNS::OutputtingProcessC& outProc){
                auto const& clients = DataForCommands.Server->gClients();
                for (auto client = ++clients.begin();client != clients.end();++client) {
                    outProc << client->Socket.remote_endpoint().address().to_string() << outProc.EndLine;
                }
            }},
            {"exit",[](ConsoleManagerNS::OutputNS::OutputtingProcessC&){
                StopReading = true;
            }}
        }; constexpr size_t CommandsAmount = sizeof(Commands) / sizeof(std::remove_array_pointer_t<decltype(Commands)>);
    }

    
    std::thread InitializeConsoleReadingThread() {
        return std::thread([&] {
            while (true) {
                ConsoleManagerNS::OutputNS::OutputtingProcessWrapperC outProc;
                while (true) {
                    char ch = ConsoleManagerNS::InputNS::ReadChar();
                    std::lock_guard ul(Mutex);
                    if (StopReading) {
                        outProc << outProc.EndLine;
                        return;
                    }
                    if (ch == '\r') {
                        outProc << outProc.EndLine;
                        break;
                    }
                    else {
                        if (ch == '\b'){
                            if (!CommandBuffer.empty()) {
                                CommandBuffer.pop_back(); outProc << "\b \b" << outProc.FlushOutput;
                            }
                        }
                        else {
                            outProc << ch << outProc.FlushOutput;
                            CommandBuffer.push_back(ch);
                        }
                    }
                }
                std::remove_array_pointer_t<decltype(ConsoleCommandsNS::CommandsNS::Commands)>* fcom = nullptr;
                for (size_t ci = 0;ci < ConsoleCommandsNS::CommandsNS::CommandsAmount;ci++) {
                    auto& com = ConsoleCommandsNS::CommandsNS::Commands[ci];
                    if (CommandBuffer.compare(0, com.first.size(), com.first) == 0) {
                        fcom = &com;
                        break;
                    }
                }
                if (fcom == nullptr) outProc << "Command not found" << outProc.EndLine;
                else {
                    /*
                    char curScrollSymbol = '/';
                    std::mutex scrollSymbolMutex;
                    std::condition_variable scrollSymbolCV;
                    bool stopScrollingSymbol = false;
                    std::thread scrollingSymbolTh([&] {
                        while (true) {
                            std::unique_lock ul(scrollSymbolMutex);
                            std::cout << '\b' << curScrollSymbol << std::flush;
                            if (curScrollSymbol == '/') curScrollSymbol = '-';
                            else if (curScrollSymbol == '-') curScrollSymbol = '\\';
                            else if (curScrollSymbol == '\\') curScrollSymbol = '|';
                            else if (curScrollSymbol == '|') curScrollSymbol = '/';
                            using namespace std::chrono_literals;
                            scrollSymbolCV.wait_for(ul, 300ms, [&] { return stopScrollingSymbol;});
                            if (stopScrollingSymbol) { std::cout << "\b \b" << std::flush; return; }
                        }
                        });*/
                    fcom->second(outProc);
                    //stopScrollingSymbol = true; scrollSymbolCV.notify_all();
                    //scrollingSymbolTh.join();
                }
                CommandBuffer.resize(0);
                if (StopReading) return;
            }
            });
    }
}