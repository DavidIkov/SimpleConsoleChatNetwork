#include"users.hpp"


int main(int argc, char** argv){
    server::UsersHandler serv;
    serv.StartListening(12333);
    std::this_thread::sleep_for(std::chrono::hours(10));
}
