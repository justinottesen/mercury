#pragma once

#include <expected>
#include <memory_resource>
#include <string_view>

#include <mercury/http/error.hpp>
#include <mercury/http/method.hpp>
#include <mercury/http/request.hpp>
#include <mercury/http/version.hpp>
#include <mercury/net/read_buffer.hpp>
#include <mercury/net/slice.hpp>

namespace mercury::http {

template <net::Readable Socket, std::size_t Cap = 8192>
[[nodiscard]] auto parseRequest(net::ReadBuffer<Socket, Cap>& buf,
                                std::pmr::memory_resource&    arena)
    -> std::expected<Request, ParseError> {

    // Copies `sv` into the arena — the one intentional copy per string.
    // When io_uring fills arena chunks directly, this goes away.
    auto intern = [&arena](std::string_view sv) -> net::Slice {
        if (sv.empty()) { return {}; }
        auto* ptr = static_cast<char*>(arena.allocate(sv.size(), alignof(char)));
        std::ranges::copy(sv, ptr);
        return net::Slice{ptr, sv.size()};
    };

    // --- Request line: METHOD SP target SP HTTP/version CRLF ---
    auto const reqLine = buf.readUntil("\r\n");
    if (!reqLine) { return std::unexpected{ParseError::IoError}; }

    auto const firstSpace  = reqLine->find(' ');
    auto const secondSpace = firstSpace != std::string_view::npos
                           ? reqLine->find(' ', firstSpace + 1)
                           : std::string_view::npos;

    if (firstSpace == std::string_view::npos || secondSpace == std::string_view::npos) {
        return std::unexpected{ParseError::InvalidRequestLine};
    }

    auto const method = method::fromString(reqLine->substr(0, firstSpace));
    if (!method) { return std::unexpected{ParseError::InvalidMethod}; }

    auto const target = intern(reqLine->substr(firstSpace + 1, secondSpace - firstSpace - 1));

    auto const version = version::fromString(reqLine->substr(secondSpace + 1));
    if (!version) { return std::unexpected{ParseError::InvalidVersion}; }

    // --- Headers: Name ": " Value CRLF, terminated by empty line ---
    std::pmr::vector<std::pair<net::Slice, net::Slice>> headers{&arena};
    while (true) {
        auto const headerLine = buf.readUntil("\r\n");
        if (!headerLine) { return std::unexpected{ParseError::IoError}; }
        if (headerLine->empty()) { break; }

        auto const sep = headerLine->find(": ");
        if (sep == std::string_view::npos) { return std::unexpected{ParseError::InvalidHeader}; }

        headers.emplace_back(intern(headerLine->substr(0, sep)),
                             intern(headerLine->substr(sep + 2)));
    }

    return Request{.method  = *method,
                   .target  = target,
                   .version = *version,
                   .headers = std::move(headers)};
}

}    // namespace mercury::http
