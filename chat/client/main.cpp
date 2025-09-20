#include <iostream>
#include <vector>

#include "events/parser/parse.hpp"
#include "spdlog/spdlog.h"
#include "user.hpp"

int main(int argc, char** argv) {
    spdlog::set_pattern("[%H:%M:%S.%e|%^%l%$|%P:%t|%s:%#] %v");
    events::ParseEventsConfig("events");
    client::UserHandler client;

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

        try {
            if (args.size() != 0) {
                if (args[0] == "exit") {
                    std::cout << "exiting" << std::endl;
                    break;
                } else if (args[0] == "connect") {
                    if (args.size() != 6)
                        std::cout << "not enough arguments" << std::endl;
                    else {
                        client.Connect(networking::Endpoint(
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
                        } else if (args[1] == "user") {
                            if (client.IsLoggedIn())
                                std::cout << client.GetUser() << std::endl;
                            else
                                std::cout << "not logged in" << std::endl;
                        } /*else if (args[1] == "room") {
                            if (client.IsInRoom())
                                std::cout << client.GetRoom() << std::endl;
                            else
                                std::cout << "not in room" << std::endl;
                        } */
                        else
                            std::cout << "unknown arguments" << std::endl;
                    }
                } else if (args[0] == "login") {
                    if (args.size() != 3)
                        std::cout << "incorrect arguments amount" << std::endl;
                    else {
                        shared::id_t id = client.GetUserIDByName(args[1]);
                        if (!id)
                            std::cout << "this user does not exist"
                                      << std::endl;
                        else
                            client.LogInUser(id, args[2]);
                    }
                } else if (args[0] == "logout") {
                    client.LogOutOfUser();
                } else if (args[0] == "register") {
                    if (args.size() != 3)
                        std::cout << "incorrect arguments amount" << std::endl;
                    else {
                        client.RegisterUser(args[1], args[2]);
                    }
                } else if (args[0] == "create") {
                    if (args.size() < 2)
                        std::cout << "incorrect arguments amount" << std::endl;
                    else {
                        if (args[1] == "room") {
                            /*if (args.size() != 4)
                                std::cout << "incorrect arguments amount"
                                          << std::endl;
                            else
                                client.CreateRoom(args[2],
                                                  args[3]);*/
                        } else {
                            std::cout << "unknown arguments" << std::endl;
                        }
                    }
                }
                /*else if (args[0] == "delete")
                        client.LeaveRoom();
                    */
                else
                    std::cout << "unknown command" << std::endl;
            }
        } catch (std::exception& err) {
            std::cout << "error: " << err.what() << std::endl;
        }
    }

    return 0;
}
