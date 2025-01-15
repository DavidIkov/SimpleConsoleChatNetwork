#include <iostream>
#define _WIN32_WINNT 0x0A00
#define ASIO_STANDALONE
#include "asio.hpp"
#include "asio/ts/buffer.hpp"
#include "asio/ts/internet.hpp"
#include <thread>
#include <chrono>

char BufferForText[100];

static std::list<asio::ip::tcp::socket> clientsSockets;
static asio::ip::tcp::endpoint curClientEndpoint;
void ResendAllData_Async(asio::ip::tcp::socket& socket) {
    socket.async_read_some(asio::buffer(BufferForText), [&](asio::error_code ec, size_t bytes) {
        if (ec) {
            if (ec == asio::error::eof) {
                std::cout << "Client disconnected, removing socket" << std::endl;
                clientsSockets.remove_if([&](auto& v)->bool {return &v == &socket;});
                return;
            }
            std::cout << "Error occured while reading data from server's socket. Ignoring current data. Exact error is: " <<
                ec.value() << ' ' << ec.message() << std::endl;
        };
        std::cout << "Server received message: ";
        for (size_t i = 0; i < bytes; i++)
            std::cout << BufferForText[i];
        std::cout << std::endl;

        socket.async_write_some(asio::buffer(BufferForText, bytes), [](asio::error_code ec, size_t bytes){std::cout << "Resended all of it\n";});
        ResendAllData_Async(socket);
        });
}
void ProcessClients_Async(asio::io_context& serverContext, asio::ip::tcp::acceptor& serverAcceptor) {
    clientsSockets.emplace_back(serverContext);
    serverAcceptor.async_accept(*clientsSockets.rbegin(), curClientEndpoint, [&](asio::error_code) {
        std::cout << curClientEndpoint.address().to_string() << std::endl;
        ResendAllData_Async(*clientsSockets.rbegin());
        ProcessClients_Async(serverContext, serverAcceptor);
        });
}

int main(int argc, char** argv)
{

    asio::error_code CurErrorCode;
    asio::io_context CurContext;

    asio::io_context::work IdleWork(CurContext);

    std::thread ContextThread([&]
        { CurContext.run(); });

    asio::ip::tcp::acceptor ServerAcceptor(CurContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 16120));

    std::cout << "Server is up" << std::endl;
    ProcessClients_Async(CurContext, ServerAcceptor);


    ContextThread.join();
    std::cout << "Finished waiting uhhhh\n";

    return 0;
}
