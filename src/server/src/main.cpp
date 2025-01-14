#include<iostream>
#define _WIN32_WINNT 0x0A00
#define ASIO_STANDALONE
#include"asio.hpp"
#include"asio/ts/buffer.hpp"
#include"asio/ts/internet.hpp"
#include<thread>
#include<chrono>

char BufferForText[100];

void PrintAllData(asio::ip::tcp::socket& socket){
    socket.async_read_some(asio::buffer(BufferForText), [&](asio::error_code ec, size_t bytesAmount) {
            if (ec == asio::error::eof)
                return;
            for (size_t i = 0; i < bytesAmount; i++)
                std::cout << BufferForText[i];
            std::cout << "\n\n\nREPEATING\n\n\n";
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(100ms);
            PrintAllData(socket); 
        });
   /*
    asio::error_code ec;
    size_t bytesAmount = socket.read_some(asio::buffer(BufferForText), ec);
    if (ec == asio::error::eof)
        return;
    for (size_t i = 0; i < bytesAmount; i++)
        std::cout << BufferForText[i];
    std::cout << "\n\n\nREPEATING\n\n\n";
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
    PrintAllData(socket);
    */
}

int main(int argc, char** argv) {

    using namespace std::chrono_literals;

    asio::error_code CurErrorCode;
    asio::io_context CurContext;

    asio::ip::tcp::acceptor ServerAcceptor(CurContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 16120));

    asio::ip::tcp::socket ServerSocket(CurContext);

    std::cout << "Server is up" << std::endl;
    
    ServerAcceptor.accept(ServerSocket);
    std::cout << ServerSocket.remote_endpoint().address().to_string() << std::endl;

    std::this_thread::sleep_for(1000ms);

    size_t bytes = ServerSocket.read_some(asio::buffer(BufferForText));
    std::cout << "Server received message: ";
    for (size_t i = 0; i < bytes;i++) std::cout << BufferForText[i];
    std::cout << std::endl;
    ServerSocket.write_some(asio::buffer("lol you dumbass"));

    std::cout << "Server just sended client some stupid ass message" << std::endl;

    std::this_thread::sleep_for(10000ms);
    std::cout << "server is closing!\n";

    /*
        asio::io_context::work IdleWork(CurContext);

        std::thread ContextThread([&] { CurContext.run(); });

        asio::ip::tcp::endpoint EndPoint(asio::ip::make_address("51.38.81.49", CurErrorCode), 80);

        asio::ip::tcp::socket CurSocket(CurContext);

        CurSocket.connect(EndPoint, CurErrorCode);

        if (!CurErrorCode && CurSocket.is_open()){
            std::cout << "Connected!!!\n";

            PrintAllData(CurSocket);

            std::string RequestText =
                "GET /index.html HTTP/1.1\r\n"
                "Host: david-barr.co.uk\r\n"
                "Connection: close\r\n\r\n";
            CurSocket.write_some(asio::buffer(RequestText.c_str(), RequestText.size()), CurErrorCode);


            std::this_thread::sleep_for(10000ms);
        }
    */

    return 0;
}

