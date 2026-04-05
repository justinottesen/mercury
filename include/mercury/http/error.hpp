#pragma once

#include <cstdint>

namespace mercury::http {

// TODO: integrate with std::error_code / std::error_category using HTTP status codes
// (e.g. InvalidRequestLine -> 400, InvalidVersion -> 505 HTTP Version Not Supported)
enum class ParseError : std::uint8_t {
    IoError,
    InvalidMethod,
    InvalidRequestLine,
    InvalidVersion,
    InvalidHeader,
};

}    // namespace mercury::http
