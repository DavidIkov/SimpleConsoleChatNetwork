#pragma once
#include"AsioInclude.hpp"
#include"ConsoleManager.hpp"
#include"ServerClass/BasicServer.hpp"
#include<string>
#include<mutex>
namespace ConsoleCommandsNS {

    namespace DataForCommandsNS {
        inline BasicServerC* Server = nullptr;
    }
    
    inline std::mutex Mutex;
    inline std::string CommandBuffer;
    inline bool StopReading = false;
    namespace CommandsNS {
        extern const size_t CommandsAmount;
        std::pair<std::string_view, void(*)(ConsoleManagerNS::OutputNS::OutputtingProcessC&)> Commands[] = {
            {"list clients",[](ConsoleManagerNS::OutputNS::OutputtingProcessC& outProc){
                auto const& clients = DataForCommandsNS::Server->gClients();
                for (auto client = ++clients.begin();client != clients.end();++client) {
                    outProc << client->get()->Socket.remote_endpoint().address().to_string() << outProc.EndLine;
                }
            }},
            {"exit",[](ConsoleManagerNS::OutputNS::OutputtingProcessC&){
                StopReading = true;
            }}
        }; inline constexpr size_t CommandsAmount = std::extent_v<decltype(Commands)>;
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
                std::remove_all_extents_t<decltype(ConsoleCommandsNS::CommandsNS::Commands)>* fcom = nullptr;
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