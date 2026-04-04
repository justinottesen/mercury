#pragma once

#include <mercury/ip/address.hpp>

namespace mercury::ip {

namespace detail {

template <AddressType AddrType>
class GenericEndpoint {
public:
    constexpr GenericEndpoint(AddrType addr, std::uint16_t port) noexcept
        : m_addr{addr}
        , m_port{port} {}

    [[nodiscard]] constexpr auto address() const noexcept -> const AddrType& { return m_addr; }

    [[nodiscard]] constexpr auto port() const noexcept -> std::uint16_t { return m_port; }

    constexpr auto operator==(const GenericEndpoint&) const noexcept -> bool = default;

private:
    AddrType      m_addr;
    std::uint16_t m_port;
};

}    // namespace detail

using EndpointV4 = detail::GenericEndpoint<AddressV4>;
using EndpointV6 = detail::GenericEndpoint<AddressV6>;
using Endpoint   = std::variant<EndpointV4, EndpointV6>;

template <typename T>
concept EndpointType =
    std::same_as<T, EndpointV4> || std::same_as<T, EndpointV6> || std::same_as<T, Endpoint>;

}    // namespace mercury::ip
