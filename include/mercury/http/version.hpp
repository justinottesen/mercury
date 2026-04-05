#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace mercury::http {

enum class Version : std::uint8_t { v1, v1_1 };

namespace version {

[[nodiscard]] constexpr auto fromString(std::string_view s) noexcept -> std::optional<Version> {
    if (s == "HTTP/1.0") { return Version::v1; }
    if (s == "HTTP/1.1") { return Version::v1_1; }
    return std::nullopt;
}

}    // namespace version

}    // namespace mercury::http
