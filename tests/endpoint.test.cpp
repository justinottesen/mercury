#include <mercury/ip/endpoint.hpp>

#include <gtest/gtest.h>

using mercury::ip::AddressV4;
using mercury::ip::AddressV6;
using mercury::ip::Endpoint;
using mercury::ip::EndpointV4;
using mercury::ip::EndpointV6;

// --- EndpointV4 Tests ---

TEST(EndpointV4Test, Construction) {
    EndpointV4 ep(AddressV4::loopback(), 8080);
    EXPECT_EQ(ep.address(), AddressV4::loopback());
    EXPECT_EQ(ep.port(), 8080);
}

TEST(EndpointV4Test, PortZero) {
    EndpointV4 ep(AddressV4::any(), 0);
    EXPECT_EQ(ep.port(), 0);
}

TEST(EndpointV4Test, PortMax) {
    EndpointV4 ep(AddressV4::any(), 65535);
    EXPECT_EQ(ep.port(), 65535);
}

TEST(EndpointV4Test, Equality) {
    EndpointV4 a(AddressV4::loopback(), 80);
    EndpointV4 b(AddressV4::loopback(), 80);
    EndpointV4 c(AddressV4::loopback(), 443);
    EndpointV4 d(AddressV4::any(), 80);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
}

// --- EndpointV6 Tests ---

TEST(EndpointV6Test, Construction) {
    EndpointV6 ep(AddressV6::loopback(), 443);
    EXPECT_EQ(ep.address(), AddressV6::loopback());
    EXPECT_EQ(ep.port(), 443);
}

TEST(EndpointV6Test, Equality) {
    EndpointV6 a(AddressV6::loopback(), 80);
    EndpointV6 b(AddressV6::loopback(), 80);
    EndpointV6 c(AddressV6::any(), 80);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

// --- Endpoint (variant) Tests ---

TEST(EndpointTest, HoldsV4) {
    Endpoint ep = EndpointV4(AddressV4::loopback(), 8080);
    ASSERT_TRUE(std::holds_alternative<EndpointV4>(ep));

    auto& v4 = std::get<EndpointV4>(ep);
    EXPECT_EQ(v4.address(), AddressV4::loopback());
    EXPECT_EQ(v4.port(), 8080);
}

TEST(EndpointTest, HoldsV6) {
    Endpoint ep = EndpointV6(AddressV6::loopback(), 443);
    ASSERT_TRUE(std::holds_alternative<EndpointV6>(ep));

    auto& v6 = std::get<EndpointV6>(ep);
    EXPECT_EQ(v6.address(), AddressV6::loopback());
    EXPECT_EQ(v6.port(), 443);
}

TEST(EndpointTest, VisitPort) {
    Endpoint v4 = EndpointV4(AddressV4::any(), 80);
    Endpoint v6 = EndpointV6(AddressV6::any(), 443);

    auto getPort = [](const auto& ep) { return ep.port(); };
    EXPECT_EQ(std::visit(getPort, v4), 80);
    EXPECT_EQ(std::visit(getPort, v6), 443);
}

// --- EndpointType Concept Tests ---

static_assert(mercury::ip::EndpointType<EndpointV4>);
static_assert(mercury::ip::EndpointType<EndpointV6>);
static_assert(mercury::ip::EndpointType<Endpoint>);
static_assert(!mercury::ip::EndpointType<int>);
static_assert(!mercury::ip::EndpointType<AddressV4>);
