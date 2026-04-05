#pragma once

#include <expected>
#include <string_view>

#include <mercury/http/error.hpp>
#include <mercury/http/method.hpp>
#include <mercury/http/request.hpp>
#include <mercury/http/version.hpp>
#include <mercury/net/read_buffer.hpp>

namespace mercury::http {

template <net::Readable Socket, std::size_t Cap = 8192>
[[nodiscard]] auto parseRequest(net::ReadBuffer<Socket, Cap>& buf)
    -> std::expected<Request, ParseError> {

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

    // Copy target before the next readUntil invalidates the view.
    auto target = std::string{reqLine->substr(firstSpace + 1, secondSpace - firstSpace - 1)};

    auto const version = version::fromString(reqLine->substr(secondSpace + 1));
    if (!version) { return std::unexpected{ParseError::InvalidVersion}; }

    // --- Headers: Name ": " Value CRLF, terminated by empty line ---
    std::vector<std::pair<std::string, std::string>> headers;
    while (true) {
        auto const headerLine = buf.readUntil("\r\n");
        if (!headerLine) { return std::unexpected{ParseError::IoError}; }
        if (headerLine->empty()) { break; }

        auto const sep = headerLine->find(": ");
        if (sep == std::string_view::npos) { return std::unexpected{ParseError::InvalidHeader}; }

        headers.emplace_back(headerLine->substr(0, sep), headerLine->substr(sep + 2));
    }

    return Request{.method  = *method,
                   .target  = std::move(target),
                   .version = *version,
                   .headers = std::move(headers)};
}

}    // namespace mercury::http
