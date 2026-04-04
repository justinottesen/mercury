#pragma once

#include <mercury/ip/address.hpp>

namespace mercury::ip {

template <Version V>
class Endpoint {
public:
    constexpr Endpoint(Address<V> addr, std::uint16_t port) noexcept
        : m_addr{addr}
        , m_port{port} {}

    [[nodiscard]] constexpr auto address() const noexcept -> const Address<V>& { return m_addr; }

    [[nodiscard]] constexpr auto port() const noexcept -> std::uint16_t { return m_port; }

    constexpr auto operator==(const Endpoint&) const noexcept -> bool = default;

private:
    Address<V>    m_addr;
    std::uint16_t m_port;
};

}    // namespace mercury::ip
