#include <mercury/socket/socket.hpp>

#include <thread>

#include <gtest/gtest.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

namespace mercury::socket::test {

namespace {

// Returns the OS-assigned port of a bound socket.
[[nodiscard]] auto boundPort(auto const& sock) -> std::uint16_t {
    sockaddr_in addr{};
    socklen_t   len = sizeof(addr);
    // NOLINTNEXTLINE(*-reinterpret-cast, bugprone-casting-through-void)
    ::getsockname(sock.nativeHandle(), reinterpret_cast<sockaddr*>(&addr), &len);
    return ntohs(addr.sin_port);
}

}    // namespace

// --- Make ---

TEST(SocketMakeTest, TcpV4) {
    auto result = Socket<Protocol::Tcp, ip::v4>::make();
    EXPECT_TRUE(result.has_value());
}

TEST(SocketMakeTest, UdpV4) {
    auto result = Socket<Protocol::Udp, ip::v4>::make();
    EXPECT_TRUE(result.has_value());
}

TEST(SocketMakeTest, TcpV6) {
    auto result = Socket<Protocol::Tcp, ip::v6>::make();
    EXPECT_TRUE(result.has_value());
}

// --- Bind ---

TEST(SocketBindTest, TcpV4LoopbackSucceeds) {
    auto sock = Socket<Protocol::Tcp, ip::v4>::make();
    ASSERT_TRUE(sock.has_value());
    auto bound = std::move(*sock).bind({ip::Address<ip::v4>::loopback(), 0});
    EXPECT_TRUE(bound.has_value());
}

TEST(SocketBindTest, TcpV4AnySucceeds) {
    auto sock = Socket<Protocol::Tcp, ip::v4>::make();
    ASSERT_TRUE(sock.has_value());
    auto bound = std::move(*sock).bind({ip::Address<ip::v4>::any(), 0});
    EXPECT_TRUE(bound.has_value());
}

TEST(SocketBindTest, TcpV4AddressAlreadyInUse) {
    auto sockA = Socket<Protocol::Tcp, ip::v4>::make();
    auto sockB = Socket<Protocol::Tcp, ip::v4>::make();
    ASSERT_TRUE(sockA.has_value());
    ASSERT_TRUE(sockB.has_value());

    auto boundA = std::move(*sockA).bind({ip::Address<ip::v4>::loopback(), 0});
    ASSERT_TRUE(boundA.has_value());

    auto const port = boundPort(*boundA);

    auto boundB = std::move(*sockB).bind({ip::Address<ip::v4>::loopback(), port});
    ASSERT_FALSE(boundB.has_value());
    EXPECT_EQ(boundB.error(), std::errc::address_in_use);
}

// --- Listen ---

TEST(SocketListenTest, TcpV4Succeeds) {
    auto sock = Socket<Protocol::Tcp, ip::v4>::make();
    ASSERT_TRUE(sock.has_value());
    auto bound = std::move(*sock).bind({ip::Address<ip::v4>::loopback(), 0});
    ASSERT_TRUE(bound.has_value());
    auto listening = std::move(*bound).listen(1);
    EXPECT_TRUE(listening.has_value());
}

// --- Move ---

TEST(SocketMoveTest, MoveConstructorTransfersOwnership) {
    auto sock = Socket<Protocol::Tcp, ip::v4>::make();
    ASSERT_TRUE(sock.has_value());
    auto moved = std::move(*sock);
    EXPECT_TRUE(std::move(moved).bind({ip::Address<ip::v4>::loopback(), 0}).has_value());
}

TEST(SocketMoveTest, MoveAssignmentTransfersOwnership) {
    auto a = Socket<Protocol::Tcp, ip::v4>::make();
    auto b = Socket<Protocol::Tcp, ip::v4>::make();
    ASSERT_TRUE(a.has_value());
    ASSERT_TRUE(b.has_value());
    *a = std::move(*b);
    EXPECT_TRUE(std::move(*a).bind({ip::Address<ip::v4>::loopback(), 0}).has_value());
}

// --- Integration ---

TEST(SocketIntegrationTest, ConnectAcceptSendRecv) {
    auto sock = Socket<Protocol::Tcp, ip::v4>::make();
    ASSERT_TRUE(sock.has_value());
    auto bound = std::move(*sock).bind({ip::Address<ip::v4>::loopback(), 0});
    ASSERT_TRUE(bound.has_value());

    auto const port = boundPort(*bound);

    auto listening = std::move(*bound).listen(1);
    ASSERT_TRUE(listening.has_value());

    constexpr std::array<std::byte, 5> message{std::byte{'h'}, std::byte{'e'}, std::byte{'l'},
                                               std::byte{'l'}, std::byte{'o'}};

    std::thread client{[port, &message]() -> void {
        auto clientSock = Socket<Protocol::Tcp, ip::v4>::make();
        EXPECT_TRUE(clientSock.has_value());
        if (!clientSock) { return; }
        auto connected = std::move(*clientSock).connect({ip::Address<ip::v4>::loopback(), port});
        EXPECT_TRUE(connected.has_value());
        if (!connected) { return; }

        auto n = connected->send(message);
        EXPECT_TRUE(n.has_value());
        EXPECT_EQ(*n, message.size());

        std::array<std::byte, message.size()> buf{};
        auto                                  m = connected->recv(buf);
        EXPECT_TRUE(m.has_value());
        EXPECT_EQ(*m, message.size());
        EXPECT_EQ(buf, message);
    }};

    auto accepted = listening->accept();
    ASSERT_TRUE(accepted.has_value());

    std::array<std::byte, message.size()> buf{};
    auto                                  n = accepted->recv(buf);
    ASSERT_TRUE(n.has_value());
    ASSERT_EQ(*n, message.size());

    auto m = accepted->send(std::span<const std::byte>{buf});
    EXPECT_TRUE(m.has_value());
    EXPECT_EQ(*m, message.size());

    client.join();
}

// --- SetOption ---

namespace {

[[nodiscard]] auto getIntOpt(auto const& sock, int level, int optname) -> int {
    int       val = -1;
    socklen_t len = sizeof(val);
    ::getsockopt(sock.nativeHandle(), level, optname, &val, &len);
    return val;
}

}    // namespace

TEST(SocketSetOptionTest, ReuseAddressEnabled) {
    auto sock = Socket<Protocol::Tcp, ip::v4>::make();
    ASSERT_TRUE(sock.has_value());
    auto result = sock->setOption(option::ReuseAddress{true});
    EXPECT_TRUE(result.has_value());
    EXPECT_NE(getIntOpt(*sock, SOL_SOCKET, SO_REUSEADDR), 0);
}

TEST(SocketSetOptionTest, ReuseAddressDisabled) {
    auto sock = Socket<Protocol::Tcp, ip::v4>::make();
    ASSERT_TRUE(sock.has_value());
    ASSERT_TRUE(sock->setOption(option::ReuseAddress{true}).has_value());
    ASSERT_TRUE(sock->setOption(option::ReuseAddress{false}).has_value());
    EXPECT_EQ(getIntOpt(*sock, SOL_SOCKET, SO_REUSEADDR), 0);
}

TEST(SocketSetOptionTest, ReusePortEnabled) {
    auto sock = Socket<Protocol::Tcp, ip::v4>::make();
    ASSERT_TRUE(sock.has_value());
    auto result = sock->setOption(option::ReusePort{true});
    EXPECT_TRUE(result.has_value());
    EXPECT_NE(getIntOpt(*sock, SOL_SOCKET, SO_REUSEPORT), 0);
}

TEST(SocketSetOptionTest, KeepAliveEnabled) {
    auto sock = Socket<Protocol::Tcp, ip::v4>::make();
    ASSERT_TRUE(sock.has_value());
    auto result = sock->setOption(option::KeepAlive{true});
    EXPECT_TRUE(result.has_value());
    EXPECT_NE(getIntOpt(*sock, SOL_SOCKET, SO_KEEPALIVE), 0);
}

TEST(SocketSetOptionTest, NoDelayEnabled) {
    auto sock = Socket<Protocol::Tcp, ip::v4>::make();
    ASSERT_TRUE(sock.has_value());
    auto result = sock->setOption(option::NoDelay{true});
    EXPECT_TRUE(result.has_value());
    EXPECT_NE(getIntOpt(*sock, IPPROTO_TCP, TCP_NODELAY), 0);
}

}    // namespace mercury::socket::test
