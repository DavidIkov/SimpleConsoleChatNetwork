#pragma once
#include <cstdint>
#include <ostream>
#include <string>

namespace Socket {

struct Endpoint {
    constexpr inline Endpoint(uint8_t octet1, uint8_t octet2, uint8_t octet3,
                              uint8_t octet4, uint16_t port)
        : ip_((uint32_t)octet1 << 24 | (uint32_t)octet2 << 16 |
              (uint32_t)octet3 << 8 | (uint32_t)octet4),
          port_(port) {}
    constexpr inline Endpoint(uint32_t ip, uint16_t port)
        : ip_(ip), port_(port) {}
    constexpr inline Endpoint() : ip_(0), port_(0) {}
    [[nodiscard]] inline std::string ip_to_string() const {
        return std::to_string(ip_ >> 24 & 0xFF) + '.' +
               std::to_string(ip_ >> 16 & 0xFF) + '.' +
               std::to_string(ip_ >> 8 & 0xFF) + '.' +
               std::to_string(ip_ & 0xFF);
    }

    uint32_t ip_;
    uint16_t port_;
};

inline bool operator<(const Endpoint& endp1, const Endpoint& endp2) {
    return (endp1.port_ == endp2.port_) ? (endp1.ip_ < endp2.ip_)
                                        : (endp1.port_ < endp2.port_);
}

inline std::ostream& operator<<(std::ostream& stream, const Endpoint& endp) {
    return stream << endp.ip_to_string() << ":" << endp.port_;
}

typedef int RawDescriptorT;
};  // namespace Socket
