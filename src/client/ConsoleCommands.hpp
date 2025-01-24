#pragma once
#include"AsioInclude.hpp"
#include"ConsoleFixedSizeInput.hpp"
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
                std::string ipStr = CommandBuffer.substr(fs + 1, ss - fs - 1);
                std::string portStr = CommandBuffer.substr(ss + 1);
                Client.Connect(asio::ip::tcp::endpoint(asio::ip::make_address(ipStr), std::stoi(portStr)));
                /*
                std::lock_guard lg(EventsNS::Mutex);
                EventsNS::EventsQueue.emplace(EventsNS::EventDataS{ EventsNS::TypesE::ConnectToServer,
                    EventsNS::ConnectToServerS{ ClientSocket,asio::ip::tcp::endpoint(asio::ip::make_address(ipStr), std::stoi(portStr)) } });
                EventsNS::CV.notify_all();
                */
            }},
            {"disconnect",[] {
                Client.Disconnect();
            }}
        }; constexpr size_t CommandsAmount = sizeof(Commands) / sizeof(std::remove_array_pointer_t<decltype(Commands)>);
    }

    
    std::thread InitializeConsoleReadingThread() {
        ConsoleFixedSizeInputC _;
        return std::thread([&] {
            while (true) {
                while (true) {
                    char ch = ConsoleFixedSizeInputC::ReadChar();
                    std::lock_guard ul(Mutex);
                    if (StopReading) {
                        std::cout << std::endl;
                        return;
                    }
                    if (ch == '\r') {
                        std::cout << std::endl; break;
                    }
                    else {
                        std::cout << ch;
                        CommandBuffer.push_back(ch);
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
                    fcom->second();
                }
                CommandBuffer.resize(0);
            }
            });
    }
}