#include "events/parser/parse.hpp"
#include "spdlog/spdlog.h"
#include "users.hpp"

int main(int argc, char** argv) {
    spdlog::set_pattern("[%H:%M:%S.%e|%^%l%$|%P:%t|%s:%#] %v");
    events::ParseEventsConfig("events");
    server::UsersHandler serv;
    serv.StartListening(12333);
    std::this_thread::sleep_for(std::chrono::hours(10));
}
