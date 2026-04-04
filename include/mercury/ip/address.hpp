#pragma once

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <expected>
#include <ranges>
#include <string_view>
#include <variant>

#include <future/concat.hpp>

namespace mercury::ip {

class AddressV4 {
    static constexpr std::size_t OCTET_COUNT = 4;

    using OctetArray = std::array<std::uint8_t, OCTET_COUNT>;

public:
    constexpr AddressV4() noexcept = default;

    explicit constexpr AddressV4(const OctetArray& octets) noexcept
        : m_octets(octets) {}

    constexpr AddressV4(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d) noexcept
        : m_octets{a, b, c, d} {}

    [[nodiscard]] static constexpr auto fromString(std::string_view str)
        -> std::expected<AddressV4, std::errc> {
        auto       parts = str | std::views::split('.');
        OctetArray octets{};

        // Parse the octets from the string
        std::size_t count = 0;
        for (auto [sv, octet] : std::views::zip(parts, octets)) {
            ++count;
            int value{};
            auto [end, ec] = std::from_chars(sv.begin(), sv.end(), value);
            if (ec != std::errc{} || end != sv.end() || !std::in_range<std::uint8_t>(value)) {
                return std::unexpected{std::errc::invalid_argument};
            }
            octet = static_cast<std::uint8_t>(value);
        }

        // If we didn't fill the array, check for a special case or return invalid
        if (count != OCTET_COUNT) {
            if (str == "localhost") { return loopback(); }
            if (str == "any") { return any(); }
            if (str == "broadcast") { return broadcast(); }
            return std::unexpected{std::errc::invalid_argument};
        }

        return AddressV4{octets};
    }

    [[nodiscard]] constexpr auto toString() const -> std::string {
        std::string result;
        for (auto [i, octet] : std::views::enumerate(m_octets)) {
            if (i > 0) { result += '.'; }
            std::array<char, 3> buf{};
            auto [end, _] = std::to_chars(buf.begin(), buf.end(), octet);
            result.append(buf.begin(), end);
        }
        return result;
    }

    [[nodiscard]] static constexpr auto any() noexcept -> AddressV4 { return {0, 0, 0, 0}; }

    [[nodiscard]] static constexpr auto loopback() noexcept -> AddressV4 { return {127, 0, 0, 1}; }

    [[nodiscard]] static constexpr auto broadcast() noexcept -> AddressV4 {
        return {255, 255, 255, 255};
    }

    constexpr auto operator==(const AddressV4&) const noexcept -> bool = default;

private:
    OctetArray m_octets{};
};

class AddressV6 {
    static constexpr std::size_t OCTET_COUNT = 16;

    using OctetArray = std::array<std::uint8_t, OCTET_COUNT>;

public:
    constexpr AddressV6() noexcept = default;

    explicit constexpr AddressV6(const OctetArray& octets) noexcept
        : m_octets(octets) {}

    [[nodiscard]] static constexpr auto fromString(std::string_view str)
        -> std::expected<AddressV6, std::errc> {
        if (str.empty()) { return std::unexpected{std::errc::invalid_argument}; }

        auto dcPos = str.find("::");

        if (dcPos == std::string_view::npos) {
            auto parsed = parseGroups(str);
            if (!parsed.valid || std::cmp_not_equal(parsed.count, GROUP_COUNT)) {
                if (str == "localhost") { return loopback(); }
                return std::unexpected{std::errc::invalid_argument};
            }
            return fromGroups(parsed.groups);
        }

        auto leftParsed  = parseGroups(str.substr(0, dcPos));
        auto rightParsed = parseGroups(str.substr(dcPos + 2));

        if (!leftParsed.valid || !rightParsed.valid) {
            return std::unexpected{std::errc::invalid_argument};
        }
        if (static_cast<std::size_t>(leftParsed.count) + static_cast<std::size_t>(rightParsed.count)
            >= GROUP_COUNT) {
            return std::unexpected{std::errc::invalid_argument};
        }

        auto leftCount  = static_cast<std::size_t>(leftParsed.count);
        auto rightCount = static_cast<std::size_t>(rightParsed.count);
        auto zeroCount  = GROUP_COUNT - leftCount - rightCount;

        std::array<std::uint16_t, GROUP_COUNT> groups{};
        std::ranges::copy(future::views::concat(leftParsed.groups | std::views::take(leftCount),
                                                std::views::repeat(std::uint16_t{}, zeroCount),
                                                rightParsed.groups | std::views::take(rightCount)),
                          groups.begin());
        return fromGroups(groups);
    }

    [[nodiscard]] constexpr auto toString() const -> std::string {
        // Extract 8 x 16-bit groups from the 16 raw bytes
        std::array<std::uint16_t, GROUP_COUNT> groups{};
        std::ranges::copy(
            m_octets | std::views::chunk(2) | std::views::transform([](auto pair) -> std::uint16_t {
                return static_cast<std::uint16_t>((static_cast<std::uint32_t>(pair[0]) << 8U)
                                                  | static_cast<std::uint32_t>(pair[1]));
            }),
            groups.begin());

        // Find the longest run of zero groups (min length 2) for :: compression
        int bestStart = -1;
        int bestLen   = 0;
        int curStart  = -1;
        int curLen    = 0;
        for (auto [i, group] : std::views::enumerate(groups)) {
            if (group == 0) {
                if (curStart == -1) {
                    curStart = static_cast<int>(i);
                    curLen   = 0;
                }
                if (++curLen > bestLen) {
                    bestLen   = curLen;
                    bestStart = curStart;
                }
            } else {
                curStart = -1;
                curLen   = 0;
            }
        }
        // RFC 5952 §4.2.2: '::' must not be used for a single zero group.
        constexpr int min_compression_len = 2;
        if (bestLen < min_compression_len) { bestStart = -1; }

        std::string result;
        bool        needColon = false;
        for (auto [i, group] : std::views::enumerate(groups)) {
            auto idx = static_cast<int>(i);
            if (bestStart != -1 && idx >= bestStart && idx < bestStart + bestLen) {
                if (idx == bestStart) {
                    result += "::";
                    needColon = false;
                }
                continue;
            }
            if (needColon) { result += ':'; }
            std::array<char, 4> buf{};
            auto [end, _] = std::to_chars(buf.begin(), buf.end(), group, 16);
            result.append(buf.begin(), end);
            needColon = true;
        }

        return result;
    }

    [[nodiscard]] static constexpr auto any() noexcept -> AddressV6 {
        return AddressV6{OctetArray{}};
    }

    [[nodiscard]] static constexpr auto loopback() noexcept -> AddressV6 {
        return AddressV6{
            OctetArray{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}
        };
    }

    auto operator==(const AddressV6&) const noexcept -> bool = default;

private:
    OctetArray m_octets{};

    static constexpr std::size_t GROUP_COUNT = OCTET_COUNT / 2;

    struct GroupsResult {
        std::array<std::uint16_t, GROUP_COUNT> groups{};
        int                                    count = 0;
        bool                                   valid = true;
    };

    [[nodiscard]] static constexpr auto parseGroups(std::string_view s) noexcept -> GroupsResult {
        GroupsResult result{};
        if (s.empty()) { return result; }

        auto parts = s | std::views::split(':');
        auto count = std::ranges::distance(parts);
        if (std::cmp_greater(count, GROUP_COUNT)) {
            result.valid = false;
            return result;
        }

        for (auto [sv, group] : std::views::zip(parts, result.groups)) {
            std::uint16_t value{};
            auto [end, ec] = std::from_chars(sv.begin(), sv.end(), value, 16);
            if (ec != std::errc{} || end != sv.end()) {
                result.valid = false;
                return result;
            }
            group = value;
        }
        result.count = static_cast<int>(count);
        return result;
    }

    [[nodiscard]] static constexpr auto fromGroups(
        const std::array<std::uint16_t, GROUP_COUNT>& groups) noexcept -> AddressV6 {
        OctetArray octets{};
        for (auto [pair, group] : std::views::zip(octets | std::views::chunk(2), groups)) {
            pair[0] = static_cast<std::uint8_t>(group >> 8U);
            pair[1] = static_cast<std::uint8_t>(group & 0xFFU);
        }
        return AddressV6{octets};
    }
};

using Address = std::variant<AddressV4, AddressV6>;

template <typename T>
concept AddressType =
    std::same_as<T, AddressV4> || std::same_as<T, AddressV6> || std::same_as<T, Address>;

}    // namespace mercury::ip
