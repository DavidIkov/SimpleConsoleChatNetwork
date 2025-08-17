#include <iostream>

#include "user.hpp"

int main(int argc, char** argv) {
    client::UserHandler client;
    client.Connect({127, 0, 12, 13, 12333});
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "logging in as david" << std::endl;
    client.Login("david", "123");
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 0;
}
