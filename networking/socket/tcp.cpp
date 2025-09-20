#include "tcp.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "spdlog/spdlog.h"

using namespace networking::socket;

TCP::TCP(TCP &&sock) noexcept : descriptor_(sock.descriptor_) {}

TCP::TCP(RawDescriptorT raw_descriptor) noexcept
    : descriptor_(raw_descriptor), connected_(true) {}

TCP &TCP::operator=(TCP &&sock) noexcept {
    descriptor_ = sock.descriptor_;
    return *this;
}

TCP::~TCP() {
    if (IsOpened()) Close();
}

void TCP::Open() {
    if (IsOpened()) {
        SPDLOG_ERROR("failed to open socket, it is already opened");
        throw std::exception();
    }
    std::lock_guard LG(mutex_);
    descriptor_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (descriptor_ == -1) {
        SPDLOG_ERROR("failed to open socket with error: {}", errno);
        throw std::exception();
    }
}

void TCP::Connect(Endpoint const &endp) {
    sockaddr_in endp_addr;
    endp_addr.sin_family = AF_INET;
    endp_addr.sin_port = htons(endp.port_);
    endp_addr.sin_addr.s_addr = htonl(endp.ip_);

    std::lock_guard LG(mutex_);

    if (connect(descriptor_, (sockaddr *)&endp_addr, sizeof(endp_addr)) == -1) {
        descriptor_ = -1;
        SPDLOG_ERROR("failed to connect socket to {} with error {}", endp,
                     errno);
        throw std::exception();
    }
    connected_ = true;
}

void TCP::BindToAddress(Endpoint const &endp) {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(endp.port_);
    addr.sin_addr.s_addr = htonl(endp.ip_);

    std::lock_guard LG(mutex_);
    if (bind(descriptor_, (sockaddr *)&addr, sizeof(addr)) == -1) {
        SPDLOG_ERROR(
            "failed to change socket local address to {} with error {}", endp,
            errno);
        throw std::exception();
    }
}

void TCP::SendData(void const *data, size_t bytes) const {
    std::lock_guard LG(mutex_);
    while (bytes) {
        size_t bytes_sent = send(descriptor_, data, bytes, MSG_NOSIGNAL);
        if (bytes_sent == -1) {
            SPDLOG_ERROR("failed to send data of length {} bytes with error {}",
                         bytes, errno);
            throw std::exception();
        }

        data = (uint8_t *)data + bytes_sent;
        bytes -= bytes_sent;
    }
}

size_t TCP::ReceiveData(void *data, size_t size) const {
    if (!size) {
        SPDLOG_ERROR("failed to read from socket, buffer is empty");
        throw std::exception();
    }
    int descriptor;
    {
        std::lock_guard LG(mutex_);
        descriptor = descriptor_;
    }
    ssize_t bytes = read(descriptor, data, size);
    if (bytes == -1) {
        if (errno == ECONNRESET)
            return 0;
        else {
            SPDLOG_ERROR("failed to read from socket with error {}", errno);
            throw std::exception();
        }
    }
    return bytes;
}

networking::RawDescriptorT TCP::AcceptConnection() const {
    int descriptor;
    {
        std::lock_guard LG(mutex_);
        descriptor = descriptor_;
    }
    int newSocket = accept(descriptor, nullptr, nullptr);
    if (newSocket == -1) {
        // TODO figure out why this was here
        // if (errno == EBADF)
        //     return 0;
        // else
        SPDLOG_ERROR("failed to accept connection with error {}", errno);
        throw std::exception();
    }
    return newSocket;
}
void TCP::MarkSocketAsListening(size_t queue_len) {
    std::lock_guard LG(mutex_);
    if (listen(descriptor_, queue_len) == -1) {
        SPDLOG_ERROR(
            "failed to make socket start listening with queue len of {} and "
            "error of {}",
            queue_len, errno);
        throw std::exception();
    }
}

void TCP::Close() {
    if (!IsOpened()) {
        SPDLOG_ERROR("failed to close socket, it is not opened");
        throw std::exception();
    }
    std::lock_guard LG(mutex_);
    if (close(descriptor_) == -1) {
        descriptor_ = -1;
        SPDLOG_ERROR("failed to close socket with error {}", errno);
        throw std::exception();
    }
    descriptor_ = -1;
    connected_ = false;
}

void TCP::ShutdownReading() {
    if (!IsOpened()) {
        SPDLOG_ERROR("failed to shutdown socket reading, socket is not opened");
        throw std::exception();
    }
    std::lock_guard LG(mutex_);
    if (shutdown(descriptor_, SHUT_RD) == -1) {
        SPDLOG_ERROR("failed to shutdown socket reading with error {}", errno);
        throw std::exception();
    }
}

void TCP::ShutdownWriting() {
    if (!IsOpened()) {
        SPDLOG_ERROR("failed to shutdown socket writing, socket is not opened");
        throw std::exception();
    }
    std::lock_guard LG(mutex_);
    if (shutdown(descriptor_, SHUT_WR) == -1) {
        SPDLOG_ERROR("failed to shutdown socket writing with error {}", errno);
        throw std::exception();
    }
}

networking::Endpoint TCP::GetLocalAddress() const {
    sockaddr_in addr;
    socklen_t len = sizeof(addr);

    std::lock_guard LG(mutex_);
    if (getsockname(descriptor_, (sockaddr *)&addr, &len) == -1) {
        SPDLOG_ERROR("failed to get socket local address with error {}", errno);
        throw std::exception();
    }

    return {ntohl(addr.sin_addr.s_addr), ntohs(addr.sin_port)};
}

networking::Endpoint TCP::GetRemoteAddress() const {
    sockaddr_in addr;
    socklen_t len = sizeof(addr);

    std::lock_guard LG(mutex_);
    if (getpeername(descriptor_, (sockaddr *)&addr, &len) == -1) {
        SPDLOG_ERROR("failed to get socket remote address with error {}",
                     errno);
        throw std::exception();
    }

    return {ntohl(addr.sin_addr.s_addr), ntohs(addr.sin_port)};
}

auto TCP::GetDescriptor() const -> RawDescriptorT {
    std::lock_guard LG(mutex_);
    return descriptor_;
}

bool TCP::IsConnected() const {
    std::lock_guard LG(mutex_);
    return connected_;
}

bool TCP::IsOpened() const {
    std::lock_guard LG(mutex_);
    return descriptor_ != -1;
}
