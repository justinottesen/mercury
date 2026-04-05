#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace mercury::http {

enum class Method : std::uint8_t { Get, Head, Post, Put, Delete, Options, Patch };

namespace method {

[[nodiscard]] constexpr auto fromString(std::string_view s) noexcept -> std::optional<Method> {
    if (s == "GET")     { return Method::Get; }
    if (s == "HEAD")    { return Method::Head; }
    if (s == "POST")    { return Method::Post; }
    if (s == "PUT")     { return Method::Put; }
    if (s == "DELETE")  { return Method::Delete; }
    if (s == "OPTIONS") { return Method::Options; }
    if (s == "PATCH")   { return Method::Patch; }
    return std::nullopt;
}

}    // namespace method

}    // namespace mercury::http
