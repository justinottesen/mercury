#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <ostream>
#include <string>
#include <string_view>

namespace mercury::net {

// Non-owning view into arena-managed memory.
// Currently backed by a single contiguous region; will be extended to support
// non-contiguous buffer chain chunks when the io_uring layer is introduced.
// All operations that currently delegate to std::string_view will be
// reimplemented as chunk-aware traversals at that point.
class Slice {
    const char* m_ptr{nullptr};
    std::size_t m_len{0};

public:
    constexpr Slice() noexcept = default;

    constexpr Slice(const char* ptr, std::size_t len) noexcept
        : m_ptr{ptr}, m_len{len} {}

    explicit constexpr Slice(std::string_view sv) noexcept
        : m_ptr{sv.data()}, m_len{sv.size()} {}

    [[nodiscard]] constexpr auto data() const noexcept -> const char* { return m_ptr; }
    [[nodiscard]] constexpr auto size() const noexcept -> std::size_t { return m_len; }
    [[nodiscard]] constexpr auto empty() const noexcept -> bool { return m_len == 0; }

    [[nodiscard]] constexpr auto begin() const noexcept -> const char* { return m_ptr; }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    [[nodiscard]] constexpr auto end() const noexcept -> const char* { return m_ptr + m_len; }

    [[nodiscard]] constexpr auto operator==(const Slice& other) const noexcept -> bool {
        return view() == other.view();
    }

    [[nodiscard]] constexpr auto operator==(std::string_view other) const noexcept -> bool {
        return view() == other;
    }

    // Explicit materialisation — always allocates and copies.
    [[nodiscard]] auto to_string() const -> std::string { return {m_ptr, m_len}; }

    friend auto operator<<(std::ostream& os, Slice s) -> std::ostream& {
        return os << s.view();
    }

private:
    // Temporary interop — only valid while backed by contiguous memory.
    [[nodiscard]] constexpr auto view() const noexcept -> std::string_view {
        return {m_ptr, m_len};
    }
};

// Case-insensitive comparison — use for HTTP header name lookups.
[[nodiscard]] inline auto iequals(Slice a, std::string_view b) -> bool {
    if (a.size() != b.size()) { return false; }
    return std::ranges::equal(a, b, [](char x, char y) noexcept -> bool {
        return std::tolower(static_cast<unsigned char>(x))
            == std::tolower(static_cast<unsigned char>(y));
    });
}

}    // namespace mercury::net
