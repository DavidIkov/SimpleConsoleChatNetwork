#include "rooms.hpp"

int main(int argc, char** argv) {
    server::RoomsHandler serv;
    {
        auto LG = serv.AquireLock();
        serv.StartListening(12333);
    }
    std::this_thread::sleep_for(std::chrono::hours(10));
}
