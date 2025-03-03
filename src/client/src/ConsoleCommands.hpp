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

                    auto err = DataForCommandsNS::Client->Connect(asio::ip::tcp::endpoint(ip, port));
                    if (err != BasicClientC::ConnectResultE::NoErrors) {
                        outProc << "Failed at connecting: ";
                        switch (err) {
                        case BasicClientC::ConnectResultE::TimedOut: outProc << "timed out"; break;
                        case BasicClientC::ConnectResultE::AccessDenied: outProc << "access denied"; break;
                        case BasicClientC::ConnectResultE::AddressIsAlreadyOccupied: outProc << "address is occupied"; break;
                        case BasicClientC::ConnectResultE::SocketAlreadyConnected: outProc << "client is already connected"; break;
                        case BasicClientC::ConnectResultE::ConnectionAbortedInMiddleWay: outProc << "connection aborted in middle way"; break;
                        case BasicClientC::ConnectResultE::ServerIsNotListeningAtThisPort: outProc << "server is not listening at this port"; break;
                        case BasicClientC::ConnectResultE::ServerIsOffline: outProc << "server is offline"; break;
                        case BasicClientC::ConnectResultE::SocketIsInProgressOfConnecting: outProc << "socket is in progress of connecting"; break;
                        case BasicClientC::ConnectResultE::NoEthernetConnection: outProc << "no ethernet connection"; break;
                        case BasicClientC::ConnectResultE::NoRouteToServer: outProc << "no route to server"; break;
                        case BasicClientC::ConnectResultE::AbortedByClient: outProc << "aborted by client"; break;
                        case BasicClientC::ConnectResultE::UnknownError: outProc << "unknown error"; break;
                        default: outProc << "uhandled unknown error"; break;
                        }
                        outProc << outProc.EndLine;
                    }
                }
                catch (std::invalid_argument&) {
                    outProc << "could not convert port string to port" << outProc.EndLine; return;
                }
            }},
            {"disconnect",[](ConsoleManagerNS::OutputNS::OutputtingProcessC& outProc) {
                auto err = DataForCommandsNS::Client->Disconnect();
                if (err == BasicClientC::DisconnectResultE::NoErrors) return;
                if (err == BasicClientC::DisconnectResultE::UnknownErrorOnSocketClosureButSuccessfullDisconnect){
                    outProc << "Disconnected with unknown error on socket closure";
                    return;
                }
                outProc << "Failed at disconnecting: ";
                switch (err) {
                case BasicClientC::DisconnectResultE::NotConnectedToAnything: outProc << "not connected to anything"; break;
                case BasicClientC::DisconnectResultE::UnknownError: outProc << "unknown error"; break;
                case BasicClientC::DisconnectResultE::OperationAborted: outProc << "operation aborted"; break;
                default: outProc << "unhandled unknown error"; break;
                }
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
                switch (DataForCommandsNS::Client->LogIn(userStr.data(), passStr.data())) {
                case ChatClientC::LogInResultE::NotConnected:
                    outProc << "cant login when client is not connected to any server" << outProc.EndLine; return;
                case ChatClientC::LogInResultE::AlreadyLogged:
                    outProc << "cant login since client is already registered in server" << outProc.EndLine; return;
                case ChatClientC::LogInResultE::FailedSendingEvent:
                    outProc << "failed sending login event" << outProc.EndLine; return;
                case ChatClientC::LogInResultE::UsernameTooLong:
                    outProc << "username is too long, max length is " << NetworkEventsNS::ClientUsernameMaxLen << outProc.EndLine; return;
                case ChatClientC::LogInResultE::PasswordTooLong:
                    outProc << "password is too long, max length is " << NetworkEventsNS::ClientPasswordMaxLen << outProc.EndLine; return;
                case ChatClientC::LogInResultE::Banned:
                    outProc<<"cant login since this user is banned"<<outProc.EndLine; return;
                case ChatClientC::LogInResultE::LoggedAsExistingUser:
                        outProc<<"logged as existing user"<<outProc.EndLine; return;
                case ChatClientC::LogInResultE::LoggedAsNewUser:
                    outProc << "logged as new user" << outProc.EndLine; return;
                case ChatClientC::LogInResultE::WrongPassword:
                    outProc << "failed to login, wrong password" << outProc.EndLine; return;
                case ChatClientC::LogInResultE::UnknownRespond:
                    outProc << "failed to login becouse server made unknown response" << outProc.EndLine; return;
                default: outProc << "unhandled error" << outProc.EndLine; return;
                }
            }},
            {"logout",[](ConsoleManagerNS::OutputNS::OutputtingProcessC& outProc) {
                ChatClientC::LogOutResultE res = DataForCommandsNS::Client->LogOut();
                switch (res) {
                case ChatClientC::LogOutResultE::NoErrors: return;
                case ChatClientC::LogOutResultE::NotConnected:
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