#pragma once
#include"AsioInclude.hpp"
#include"RemoveArrayPointer.hpp"
#include"ConsoleManager.hpp"
#include"ClientClass/BasicClient.hpp"
#include<string>
#include<mutex>
namespace ConsoleCommandsNS {

    struct {
        
        BasicClientC* Client = nullptr;
    } DataForCommands;

    inline std::mutex Mutex;
    inline std::string CommandBuffer;
    inline bool StopReading = false;
    namespace CommandsNS {
        inline std::pair<std::string_view, void(*)(ConsoleManagerNS::OutputNS::OutputtingProcessC&)> Commands[] = {
            {"connect",[](ConsoleManagerNS::OutputNS::OutputtingProcessC& outProc){
                BasicClientC::OutputtingProcPtrWrapperC clientOutProcWrap(*DataForCommands.Client, outProc);
                size_t fs = CommandBuffer.find_first_of(' ');
                size_t ss = CommandBuffer.find_first_of(' ', fs + 1);
                if (fs == std::string::npos || ss == std::string::npos) {
                    outProc << "wrong command formatting, should be \"connect ip port\"" << outProc.EndLine;
                    return;
                }
                std::string ipStr = CommandBuffer.substr(fs + 1, ss - fs - 1);
                std::string portStr = CommandBuffer.substr(ss + 1);
                try {
                    int port = std::stoi(portStr);
                    asio::error_code ec;
                    auto ip = asio::ip::make_address(ipStr, ec);
                    if (ec) { outProc << "could not convert ip string to ip" << outProc.EndLine; return; }
                    DataForCommands.Client->Connect(asio::ip::tcp::endpoint(ip, port));
                }
                catch (std::invalid_argument&) {
                    outProc << "could not convert port string to port" << outProc.EndLine; return;
                }
            }},
            {"disconnect",[](ConsoleManagerNS::OutputNS::OutputtingProcessC& outProc) {
                BasicClientC::OutputtingProcPtrWrapperC clientOutProcWrap(*DataForCommands.Client, outProc);
                DataForCommands.Client->Disconnect();
            }},
            {"exit",[](ConsoleManagerNS::OutputNS::OutputtingProcessC&){
                StopReading = true;
            }}
        }; inline constexpr size_t CommandsAmount = sizeof(Commands) / sizeof(std::remove_array_pointer_t<decltype(Commands)>);
    }

    
    inline std::thread InitializeConsoleReadingThread() {
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