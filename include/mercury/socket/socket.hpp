#pragma once

#include <cstdint>
#include <expected>
#include <span>
#include <utility>

#include <mercury/ip/endpoint.hpp>
#include <mercury/socket/driver.hpp>
#include <mercury/socket/option.hpp>
#include <mercury/socket/protocol.hpp>

namespace mercury::socket {

enum class State : std::uint8_t {
    Invalid   = 0,
    Bound     = 1,
    Listening = 2,
    Connected = 4,
};

constexpr auto operator|(State a, State b) noexcept -> State {
    return static_cast<State>(static_cast<std::uint8_t>(a) | static_cast<std::uint8_t>(b));
}

constexpr auto has(State s, State flag) noexcept -> bool {
    return (static_cast<std::uint8_t>(s) & static_cast<std::uint8_t>(flag)) != 0;
}

template <typename Opt>
concept SocketOption = requires(detail::Driver::Handle fd, Opt opt) {
    { detail::Driver::setOption(fd, opt) } -> std::same_as<std::expected<void, std::errc>>;
};

template <Protocol P, ip::Version V, State S = State::Invalid>
class Socket {
    using Driver = detail::Driver;

    template <Protocol, ip::Version, State>
    friend class Socket;

    explicit Socket(Driver::Handle fd) noexcept
        : m_handle{fd} {}

public:
    Socket(const Socket&)                    = delete;
    auto operator=(const Socket&) -> Socket& = delete;

    Socket(Socket&& other) noexcept
        : m_handle{std::exchange(other.m_handle, Driver::invalid_handle)} {}

    auto operator=(Socket&& other) noexcept -> Socket& {
        if (this != &other) {
            if (m_handle != Driver::invalid_handle) { Driver::close(m_handle); }
            m_handle = std::exchange(other.m_handle, Driver::invalid_handle);
        }
        return *this;
    }

    ~Socket() {
        if (m_handle != Driver::invalid_handle) { Driver::close(m_handle); }
    }

    [[nodiscard]] auto nativeHandle() const noexcept -> Driver::Handle { return m_handle; }

    template <SocketOption Opt>
    [[nodiscard]] auto setOption(Opt opt) noexcept -> std::expected<void, std::errc> {
        return Driver::setOption(m_handle, opt);
    }

    [[nodiscard]] static auto make() noexcept -> std::expected<Socket, std::errc> {
        auto result = Driver::make(P, V);
        if (!result) { return std::unexpected{result.error()}; }
        return Socket{*result};
    }

    [[nodiscard]] auto bind(
        ip::Endpoint<V> ep) && -> std::expected<Socket<P, V, S | State::Bound>, std::errc>
        requires(!has(S, State::Bound))
    {
        auto result = Driver::bind(m_handle, ep);
        if (!result) { return std::unexpected{result.error()}; }
        return Socket<P, V, S | State::Bound>{std::exchange(m_handle, Driver::invalid_handle)};
    }

    [[nodiscard]] auto listen(
        int backlog) && -> std::expected<Socket<P, V, S | State::Listening>, std::errc>
        requires(P == Protocol::Tcp && has(S, State::Bound) && !has(S, State::Listening))
    {
        auto result = Driver::listen(m_handle, backlog);
        if (!result) { return std::unexpected{result.error()}; }
        return Socket<P, V, S | State::Listening>{std::exchange(m_handle, Driver::invalid_handle)};
    }

    [[nodiscard]] auto accept() const -> std::expected<Socket<P, V, State::Connected>, std::errc>
        requires(P == Protocol::Tcp && has(S, State::Listening))
    {
        auto result = Driver::accept(m_handle);
        if (!result) { return std::unexpected{result.error()}; }
        return Socket<P, V, State::Connected>{*result};
    }

    [[nodiscard]] auto connect(
        ip::Endpoint<V> ep) && -> std::expected<Socket<P, V, State::Connected>, std::errc>
        requires(!has(S, State::Connected) && !has(S, State::Listening))
    {
        auto result = Driver::connect(m_handle, ep);
        if (!result) { return std::unexpected{result.error()}; }
        return Socket<P, V, State::Connected>{std::exchange(m_handle, Driver::invalid_handle)};
    }

    [[nodiscard]] auto send(std::span<const std::byte> data) const noexcept
        -> std::expected<std::size_t, std::errc>
        requires(has(S, State::Connected))
    {
        return Driver::send(m_handle, data);
    }

    [[nodiscard]] auto recv(std::span<std::byte> buf) const noexcept
        -> std::expected<std::size_t, std::errc>
        requires(has(S, State::Connected))
    {
        return Driver::recv(m_handle, buf);
    }

private:
    Driver::Handle m_handle{Driver::invalid_handle};
};

}    // namespace mercury::socket
