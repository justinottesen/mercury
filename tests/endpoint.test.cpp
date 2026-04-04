#include <mercury/ip/endpoint.hpp>

#include <gtest/gtest.h>

namespace mercury::ip::test {

// --- EndpointV4 Tests ---

TEST(EndpointV4Test, Construction) {
    Endpoint ep(Address<v4>::loopback(), 8080);
    EXPECT_EQ(ep.address(), Address<v4>::loopback());
    EXPECT_EQ(ep.port(), 8080);
}

TEST(EndpointV4Test, PortZero) {
    Endpoint ep(Address<v4>::any(), 0);
    EXPECT_EQ(ep.port(), 0);
}

TEST(EndpointV4Test, PortMax) {
    Endpoint ep(Address<v4>::any(), 65535);
    EXPECT_EQ(ep.port(), 65535);
}

TEST(EndpointV4Test, Equality) {
    Endpoint a(Address<v4>::loopback(), 80);
    Endpoint b(Address<v4>::loopback(), 80);
    Endpoint c(Address<v4>::loopback(), 443);
    Endpoint d(Address<v4>::any(), 80);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
}

// --- EndpointV6 Tests ---

TEST(EndpointV6Test, Construction) {
    Endpoint ep(Address<v6>::loopback(), 443);
    EXPECT_EQ(ep.address(), Address<v6>::loopback());
    EXPECT_EQ(ep.port(), 443);
}

TEST(EndpointV6Test, Equality) {
    Endpoint a(Address<v6>::loopback(), 80);
    Endpoint b(Address<v6>::loopback(), 80);
    Endpoint c(Address<v6>::any(), 80);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

}    // namespace mercury::ip::test
