#include <iostream>
#include <vector>

#include "room.hpp"

int main(int argc, char** argv) {
    client::RoomHandler client;

    std::string commandBuff;
    while (1) {
        std::cout << ">> " << std::flush;

        std::getline(std::cin, commandBuff);
        std::vector<std::string> args;
        {
            size_t ind = 0;
            while (ind != commandBuff.size()) {
                size_t char_ind = commandBuff.find_first_not_of(' ', ind);
                if (char_ind == commandBuff.npos) break;
                ind = commandBuff.find_first_of(' ', char_ind);
                if (ind == commandBuff.npos) ind = commandBuff.size();
                args.push_back(commandBuff.substr(char_ind, ind - char_ind));
            }
        }

        auto LG = client.AquireLock();

        try {
            if (args.size() != 0) {
                if (args[0] == "exit") {
                    std::cout << "exiting" << std::endl;
                    break;
                } else if (args[0] == "connect") {
                    if (args.size() != 6)
                        std::cout << "not enough arguments" << std::endl;
                    else {
                        client.Connect(Socket::Endpoint(
                            std::stoi(args[1]), std::stoi(args[2]),
                            std::stoi(args[3]), std::stoi(args[4]),
                            std::stoi(args[5])));
                        std::cout << "connected" << std::endl;
                    }
                } else if (args[0] == "disconnect") {
                    client.Disconnect();
                } else if (args[0] == "get") {
                    if (args.size() == 1)
                        std::cout << "incorrect arguments amount" << std::endl;
                    else {
                        if (args[1] == "remote") {
                            if (client.IsConnected())
                                std::cout << client.GetRemoteAddress()
                                          << std::endl;
                            else
                                std::cout << "no remote" << std::endl;
                        } else if (args[1] == "local") {
                            if (client.IsConnected())
                                std::cout << client.GetLocalAddress()
                                          << std::endl;
                            else
                                std::cout << "no remote" << std::endl;
                        } else if (args[1] == "username") {
                            if (client.IsLoggedIn())
                                std::cout << client.GetUser() << std::endl;
                            else
                                std::cout << "not logged in" << std::endl;
                        } else if (args[1] == "room") {
                            if (client.IsInRoom())
                                std::cout << client.GetRoom() << std::endl;
                            else
                                std::cout << "not in room" << std::endl;
                        } else
                            std::cout << "unknown arguments" << std::endl;
                    }
                } else if (args[0] == "login") {
                    if (args.size() != 3)
                        std::cout << "incorrect arguments amount" << std::endl;
                    else {
                        client.Login(args[1].c_str(), args[2].c_str());
                    }
                } else if (args[0] == "logout") {
                    client.Logout();
                } else if (args[0] == "join") {
                    if (args.size() != 3)
                        std::cout << "incorrect arguments amount" << std::endl;
                    else
                        client.JoinRoom(args[1].c_str(), args[2].c_str());
                } else if (args[0] == "leave")
                    client.LeaveRoom();
                else
                    std::cout << "unknown command" << std::endl;
            }
        } catch (std::exception& err) {
            std::cout << "error: " << err.what() << std::endl;
        }
    }

    {
        auto LG = client.AquireLock();
        client.StopThreads();
    }

    return 0;
}
