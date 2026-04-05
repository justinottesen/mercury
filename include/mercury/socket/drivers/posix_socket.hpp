#pragma once

#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <expected>
#include <span>

#include <mercury/ip/endpoint.hpp>
#include <mercury/socket/option.hpp>
#include <mercury/socket/protocol.hpp>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#include "mercury/ip/address.hpp"

namespace mercury::socket::detail {

struct PosixDriver {
    template <typename Opt>
    struct OptionTraits;

    struct Native {
        using Handle = int;

        [[nodiscard]] static constexpr auto ipv(ip::Version v) {
            switch (v) {
                case ip::v4: return AF_INET;
                case ip::v6: return AF_INET6;
            }
        }

        [[nodiscard]] static constexpr auto protocol(Protocol p) {
            switch (p) {
                case Protocol::Tcp: return SOCK_STREAM;
                case Protocol::Udp: return SOCK_DGRAM;
            }
        }

        template <ip::Version V>
        [[nodiscard]] static auto toSockaddr(ip::Endpoint<V> ep) noexcept {
            if constexpr (V == ip::v4) {
                sockaddr_in addr{};
                addr.sin_family = static_cast<sa_family_t>(ipv(V));
                addr.sin_port   = htons(ep.port());
                std::memcpy(&addr.sin_addr, ep.address().bytes().data(), sizeof(addr.sin_addr));
                return addr;
            } else {
                sockaddr_in6 addr{};
                addr.sin6_family = static_cast<sa_family_t>(ipv(V));
                addr.sin6_port   = htons(ep.port());
                std::memcpy(&addr.sin6_addr, ep.address().bytes().data(), sizeof(addr.sin6_addr));
                return addr;
            }
        }
    };

    using Handle = Native::Handle;
    static constexpr Handle invalid_handle = -1;

    [[nodiscard]] static auto make(Protocol p, ip::Version v) noexcept
        -> std::expected<Native::Handle, std::errc> {
        Native::Handle const fd = ::socket(Native::ipv(v), Native::protocol(p), 0);
        if (fd == -1) { return std::unexpected{static_cast<std::errc>(errno)}; }
        return fd;
    }

    static auto close(Native::Handle fd) noexcept -> void { static_cast<void>(::close(fd)); }

    template <ip::Version V>
    [[nodiscard]] static auto bind(Native::Handle fd, ip::Endpoint<V> ep) noexcept
        -> std::expected<void, std::errc> {
        auto addr = Native::toSockaddr(ep);
        // NOLINTNEXTLINE(*-reinterpret-cast, bugprone-casting-through-void)
        if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
            return std::unexpected{static_cast<std::errc>(errno)};
        }
        return {};
    }

    [[nodiscard]] static auto listen(Native::Handle fd, int backlog) noexcept
        -> std::expected<void, std::errc> {
        if (::listen(fd, backlog) == -1) { return std::unexpected{static_cast<std::errc>(errno)}; }
        return {};
    }

    [[nodiscard]] static auto accept(Native::Handle fd) noexcept
        -> std::expected<Native::Handle, std::errc> {
        Native::Handle const client = ::accept(fd, nullptr, nullptr);
        if (client == -1) { return std::unexpected{static_cast<std::errc>(errno)}; }
        return client;
    }

    template <ip::Version V>
    [[nodiscard]] static auto connect(Native::Handle fd, ip::Endpoint<V> ep) noexcept
        -> std::expected<void, std::errc> {
        auto addr = Native::toSockaddr(ep);
        // NOLINTNEXTLINE(*-reinterpret-cast, bugprone-casting-through-void)
        if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
            return std::unexpected{static_cast<std::errc>(errno)};
        }
        return {};
    }

    [[nodiscard]] static auto send(Native::Handle fd, std::span<const std::byte> data) noexcept
        -> std::expected<std::size_t, std::errc> {
        auto const n = ::send(fd, data.data(), data.size(), 0);
        if (n == -1) { return std::unexpected{static_cast<std::errc>(errno)}; }
        return static_cast<std::size_t>(n);
    }

    [[nodiscard]] static auto recv(Native::Handle fd, std::span<std::byte> buf) noexcept
        -> std::expected<std::size_t, std::errc> {
        auto const n = ::recv(fd, buf.data(), buf.size(), 0);
        if (n == -1) { return std::unexpected{static_cast<std::errc>(errno)}; }
        return static_cast<std::size_t>(n);
    }

    template <typename Opt>
    [[nodiscard]] static auto setOption(Native::Handle fd, Opt opt) noexcept
        -> std::expected<void, std::errc> {
        int const val = opt.value ? 1 : 0;
        if (::setsockopt(fd, OptionTraits<Opt>::level, OptionTraits<Opt>::name, &val,
                         sizeof(val)) == -1) {
            return std::unexpected{static_cast<std::errc>(errno)};
        }
        return {};
    }
};

template <>
struct PosixDriver::OptionTraits<option::ReuseAddress> {
    static constexpr int level = SOL_SOCKET;
    static constexpr int name  = SO_REUSEADDR;
};

template <>
struct PosixDriver::OptionTraits<option::ReusePort> {
    static constexpr int level = SOL_SOCKET;
    static constexpr int name  = SO_REUSEPORT;
};

template <>
struct PosixDriver::OptionTraits<option::KeepAlive> {
    static constexpr int level = SOL_SOCKET;
    static constexpr int name  = SO_KEEPALIVE;
};

template <>
struct PosixDriver::OptionTraits<option::NoDelay> {
    static constexpr int level = IPPROTO_TCP;
    static constexpr int name  = TCP_NODELAY;
};

}    // namespace mercury::socket::detail
