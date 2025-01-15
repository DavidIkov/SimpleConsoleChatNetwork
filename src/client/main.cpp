#include<iostream>
#define _WIN32_WINNT 0x0A00
#define ASIO_STANDALONE
#include"asio.hpp"
#include"asio/ts/buffer.hpp"
#include"asio/ts/internet.hpp"
#include<thread>
#include<chrono>

char BufferForText[100];

int main(int argc, char** argv) {

    using namespace std::chrono_literals;

    asio::error_code CurErrorCode;
    asio::io_context CurContext;

    asio::ip::tcp::socket ClientSocket(CurContext);
    CurErrorCode = ClientSocket.connect(asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1", CurErrorCode), 16120), CurErrorCode);

    if(!CurErrorCode && ClientSocket.is_open()){
        std::cout << "Client connected!\n";
        ClientSocket.write_some(asio::buffer("hello from little client"));
        std::this_thread::sleep_for(100ms);
        size_t bytes = ClientSocket.read_some(asio::buffer(BufferForText));
        std::cout << "Client receiver a message: ";
        for (size_t i = 0; i < bytes;i++)
            std::cout << BufferForText[i];
        std::cout << std::endl;
        std::this_thread::sleep_for(5000ms);

        std::cout << "client is disconnecting! bye bye\n";
        ClientSocket.close();//not necessary, destructor already doing this
    }
    else
        std::cout << "client failed to connect :(\n";
    std::this_thread::sleep_for(5000ms);

    return 0;
}

