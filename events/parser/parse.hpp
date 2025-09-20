#pragma once

#include <filesystem>

namespace events {

// can be called multiple times to add events into pool
void ParseEventsConfig(std::filesystem::path const& config);

};  // namespace events
