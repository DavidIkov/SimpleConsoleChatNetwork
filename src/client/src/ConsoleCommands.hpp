#pragma once
#include"AsioInclude.hpp"

#include"ConsoleManager.hpp"
#include<string>
#include<mutex>
namespace ConsoleCommandsNS {
    std::mutex Mutex;
    std::string CommandBuffer;
    bool StopReading = false;
    namespace CommandsNS {
        std::pair<std::string_view, void(*)()> Commands[] = {
            {"connect",[] {
                size_t fs = CommandBuffer.find_first_of(' ');
                size_t ss = CommandBuffer.find_first_of(' ', fs + 1);
                if (fs == std::string::npos || ss == std::string::npos) {
                    std::cout << "wrong command formatting, should be \"connect ip port\"" << std::endl;
                    return;
                }
                std::string ipStr = CommandBuffer.substr(fs + 1, ss - fs - 1);
                std::string portStr = CommandBuffer.substr(ss + 1);
                try {
                    int port = std::stoi(portStr);
                    asio::error_code ec;
                    auto ip = asio::ip::make_address(ipStr, ec);
                    if (ec) { std::cout << "could not convert ip string to ip" << std::endl; return; }
                    Client.Connect(asio::ip::tcp::endpoint(ip, port));
                }
                catch (std::invalid_argument&) {
                    std::cout << "could not convert port string to port" << std::endl;; return;
                }
            }},
            {"disconnect",[] {
                Client.Disconnect();
            }},
            {"exit",[] {
                StopReading = true;
            }}
        }; constexpr size_t CommandsAmount = sizeof(Commands) / sizeof(std::remove_array_pointer_t<decltype(Commands)>);
    }

    
    std::thread InitializeConsoleReadingThread() {
        ConsoleManagerNS::Initialize();
        return std::thread([&] {
            while (true) {
                while (true) {
                    char ch = ConsoleManagerNS::InputNS::ReadChar();
                    std::lock_guard ul(Mutex);
                    if (StopReading) {
                        std::cout << std::endl;
                        return;
                    }
                    if (ch == '\r') {
                        std::cout << std::endl; break;
                    }
                    else {
                        if (ch == '\b'){
                            if (!CommandBuffer.empty()) {
                                CommandBuffer.pop_back(); std::cout << "\b \b" << std::flush;
                            }
                        }
                        else {
                            std::cout << ch << std::flush;
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
                if (fcom == nullptr) std::cout << "Command not found" << std::endl;
                else {
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
                        });
                    fcom->second();
                    stopScrollingSymbol = true; scrollSymbolCV.notify_all();
                    scrollingSymbolTh.join();
                }
                CommandBuffer.resize(0);
                if (StopReading) return;
            }
            });
    }
}