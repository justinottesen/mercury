#pragma once

#include <algorithm>
#include <memory_resource>
#include <string_view>

#include <mercury/net/slice.hpp>

namespace mercury::http::detail {

// Copies `sv` into the arena and returns a Slice pointing into it.
// This is the single intentional copy per string during parsing.
// When io_uring fills arena chunks directly, this function goes away.
[[nodiscard]] inline auto intern(std::string_view sv, std::pmr::memory_resource& arena)
    -> net::Slice {
    if (sv.empty()) { return {}; }
    auto* ptr = static_cast<char*>(arena.allocate(sv.size(), alignof(char)));
    std::ranges::copy(sv, ptr);
    return net::Slice{ptr, sv.size()};
}

}    // namespace mercury::http::detail
