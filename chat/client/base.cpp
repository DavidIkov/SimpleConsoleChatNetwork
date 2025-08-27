#include "base.hpp"

#include <iostream>

namespace client {

void Base::_OnEvent(EventData const&) {
    std::cout << "received unhandled event" << std::endl;
}

}  // namespace client
