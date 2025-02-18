#pragma once
#include"AsioInclude.hpp"
#include"ConsoleManager.hpp"
#include"ClientClass/ChatClient.hpp"
#include<string>
#include<mutex>
namespace ConsoleCommandsNS {

    namespace DataForCommandsNS {
        inline ChatClientC* Client = nullptr;
    }

    inline std::mutex Mutex;
    inline std::string CommandBuffer;
    inline bool StopReading = false;
    namespace CommandsNS {
        extern const size_t CommandsAmount;
        inline std::pair<std::string_view, void(*)(ConsoleManagerNS::OutputNS::OutputtingProcessC&)> Commands[] = {
            {"connect",[](ConsoleManagerNS::OutputNS::OutputtingProcessC& outProc) {
                BasicClientC::OutputtingProcPtrWrapperC clientOutProcWrap(*DataForCommandsNS::Client, outProc);
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
                    DataForCommandsNS::Client->Connect(asio::ip::tcp::endpoint(ip, port));
                }
                catch (std::invalid_argument&) {
                    outProc << "could not convert port string to port" << outProc.EndLine; return;
                }
            }},
            {"disconnect",[](ConsoleManagerNS::OutputNS::OutputtingProcessC& outProc) {
                BasicClientC::OutputtingProcPtrWrapperC clientOutProcWrap(*DataForCommandsNS::Client, outProc);
                DataForCommandsNS::Client->Disconnect();
            }},
            {"login",[](ConsoleManagerNS::OutputNS::OutputtingProcessC& outProc) {
                size_t fs = CommandBuffer.find_first_of(' ');
                size_t ss = CommandBuffer.find_first_of(' ', fs + 1);
                if (fs == std::string::npos || ss == std::string::npos) {
                    outProc << "wrong command formatting, should be \"login username password\", where username and password dont contain spaces"
                        << outProc.EndLine;
                    return;
                }
                std::string userStr = CommandBuffer.substr(fs + 1, ss - fs - 1);
                std::string passStr = CommandBuffer.substr(ss + 1);
                ChatClientC::LogInResultE res = DataForCommandsNS::Client->LogIn(userStr.data(), passStr.data());
                switch (res) {
                case ChatClientC::LogInResultE::NoErrors: return;
                case ChatClientC::LogInResultE::NotConnectedToServer:
                    outProc << "cant login when client is not connected to any server" << outProc.EndLine; return;
                case ChatClientC::LogInResultE::AlreadyLogged:
                    outProc << "cant login since client is already registered in server" << outProc.EndLine; return;
                case ChatClientC::LogInResultE::UsernameTooLong:
                    outProc << "username is too long, max length is " << NetworkEventsNS::ClientUsernameMaxLen << outProc.EndLine; return;
                case ChatClientC::LogInResultE::PasswordTooLong:
                    outProc << "password is too long, max length is " << NetworkEventsNS::ClientPasswordMaxLen << outProc.EndLine; return;
                default: outProc << "unhandled error" << outProc.EndLine; return;
                }
            }},
            {"logout",[](ConsoleManagerNS::OutputNS::OutputtingProcessC& outProc) {
                ChatClientC::LogOutResultE res = DataForCommandsNS::Client->LogOut();
                switch (res) {
                case ChatClientC::LogOutResultE::NoErrors: return;
                case ChatClientC::LogOutResultE::NotConnectedToServer:
                    outProc << "client is not connected to any server" << outProc.EndLine; return;
                case ChatClientC::LogOutResultE::NotLoggedIn:
                    outProc << "client is not registered as any user" << outProc.EndLine; return;
                }
            }},
            {"exit",[](ConsoleManagerNS::OutputNS::OutputtingProcessC&) {
                StopReading = true;
            }},
            { "help",[](ConsoleManagerNS::OutputNS::OutputtingProcessC& outProc) {
                for (size_t i = 0;i < CommandsAmount;i++)
                    if (Commands[i].first != "help") outProc << Commands[i].first << outProc.EndLine;
            }}
        }; inline constexpr size_t CommandsAmount = std::extent_v<decltype(Commands)>;
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
                    char curScrollSymbol = '/';
                    std::mutex scrollSymbolMutex;
                    std::condition_variable scrollSymbolCV;
                    bool stopScrollingSymbol = false;
                    bool longCommand = false;
                    std::thread scrollingSymbolTh([&] {
                        {
                            std::unique_lock ul(scrollSymbolMutex);
                            //if command takes more than two seconds
                            scrollSymbolCV.wait_for(ul, std::chrono::seconds(2), [&] { return stopScrollingSymbol; });
                            if (stopScrollingSymbol) return;
                            longCommand = true;
                        }
                        while (true) {
                            std::unique_lock ul(scrollSymbolMutex);
                            outProc << curScrollSymbol << '\b' << outProc.FlushOutput;
                            if (curScrollSymbol == '/') curScrollSymbol = '-';
                            else if (curScrollSymbol == '-') curScrollSymbol = '\\';
                            else if (curScrollSymbol == '\\') curScrollSymbol = '|';
                            else if (curScrollSymbol == '|') curScrollSymbol = '/';
                            using namespace std::chrono_literals;
                            scrollSymbolCV.wait_for(ul, 300ms, [&] { return stopScrollingSymbol;});
                            if (stopScrollingSymbol) { outProc << " \bCommand finished!" << outProc.FlushOutput; return; }
                        }
                        });
                    fcom->second(outProc);
                    {
                        std::lock_guard lg(scrollSymbolMutex);
                        stopScrollingSymbol = true; scrollSymbolCV.notify_all();
                    }
                    scrollingSymbolTh.join();
                }
                CommandBuffer.resize(0);
                if (StopReading) return;
            }
            });
    }
}