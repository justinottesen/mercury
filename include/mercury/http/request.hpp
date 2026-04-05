#pragma once

#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include <mercury/http/method.hpp>
#include <mercury/http/version.hpp>
#include <mercury/net/slice.hpp>

namespace mercury::http {

struct Request {
    Method                                              method;
    net::Slice                                          target;
    Version                                             version;
    std::pmr::vector<std::pair<net::Slice, net::Slice>> headers;
};

// Case-insensitive header lookup (HTTP header names are case-insensitive).
[[nodiscard]] inline auto findHeader(Request const& req, std::string_view name)
    -> std::optional<net::Slice> {
    for (auto const& [headerName, value] : req.headers) {
        if (net::iequals(headerName, name)) { return value; }
    }
    return std::nullopt;
}

}    // namespace mercury::http
