#pragma once
#include <cstdint>
#include <ostream>
#include <string>

#include "spdlog/fmt/bundled/format.h"

namespace networking {

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

    inline std::string ToString() const {
        return fmt::format("{}:{}", ip_to_string(), port_);
    }

    uint32_t ip_;
    uint16_t port_;
};

typedef int RawDescriptorT;
};  // namespace networking

inline std::ostream& operator<<(std::ostream& stream,
                                const networking::Endpoint& endp) {
    return stream << endp.ToString();
}

template <>
struct fmt::formatter<networking::Endpoint> : fmt::formatter<std::string> {
    inline auto format(const networking::Endpoint& endpoint,
                       fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(endpoint.ToString(), ctx);
    }
};
