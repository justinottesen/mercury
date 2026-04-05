#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <expected>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace mercury::net {

template <typename T>
concept Readable = requires(T const& t, std::span<std::byte> buf) {
    { t.recv(buf) } -> std::same_as<std::expected<std::size_t, std::errc>>;
};

template <Readable Socket, std::size_t Capacity = 8192>
class ReadBuffer {
    Socket                          m_socket;
    std::array<std::byte, Capacity> m_buf{};
    std::size_t                     m_begin{0};
    std::size_t                     m_end{0};
    std::string                     m_line;

    auto compact() noexcept -> void {
        if (m_begin == 0) { return; }
        std::ranges::copy(std::span{m_buf}.subspan(m_begin, m_end - m_begin), m_buf.begin());
        m_end -= m_begin;
        m_begin = 0;
    }

    [[nodiscard]] auto fill() -> std::expected<void, std::errc> {
        if (m_end == Capacity) { compact(); }
        if (m_end == Capacity) { return std::unexpected{std::errc::no_buffer_space}; }
        auto result = m_socket.recv(std::span<std::byte>{m_buf.data() + m_end, Capacity - m_end});
        if (!result) { return std::unexpected{result.error()}; }
        m_end += *result;
        return {};
    }

public:
    explicit ReadBuffer(Socket socket) noexcept
        : m_socket{std::move(socket)} {}

    // Read until `delim` is found; returns a view of everything before the delimiter.
    // The delimiter is consumed but not included. The view is valid until the next
    // call to readUntil.
    [[nodiscard]] auto readUntil(std::string_view delim)
        -> std::expected<std::string_view, std::errc> {
        while (true) {
            // NOLINTNEXTLINE(*-reinterpret-cast)
            auto const* chars = reinterpret_cast<const char*>(m_buf.data() + m_begin);
            auto const  view  = std::string_view{chars, m_end - m_begin};
            if (auto const pos = view.find(delim); pos != std::string_view::npos) {
                m_line.assign(view.substr(0, pos));
                m_begin += pos + delim.size();
                return std::string_view{m_line};
            }
            if (auto r = fill(); !r) { return std::unexpected{r.error()}; }
        }
    }

    // Read exactly `n` bytes.
    [[nodiscard]] auto readExact(std::size_t n)
        -> std::expected<std::vector<std::byte>, std::errc> {
        while (m_end - m_begin < n) {
            if (auto r = fill(); !r) { return std::unexpected{r.error()}; }
        }
        auto result =
            std::span{m_buf}.subspan(m_begin, n) | std::ranges::to<std::vector<std::byte>>();
        m_begin += n;
        return result;
    }
};

}    // namespace mercury::net
