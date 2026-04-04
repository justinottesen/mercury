#include <mercury/ip/address.hpp>

#include <gtest/gtest.h>

using mercury::ip::AddressV4;
using mercury::ip::AddressV6;

// --- AddressV4 Tests ---

TEST(AddressV4Test, DefaultConstructor) {
    AddressV4 addr;
    EXPECT_EQ(addr, AddressV4::any());
}

TEST(AddressV4Test, OctetConstructor) {
    AddressV4 addr(192, 168, 1, 1);
    EXPECT_EQ(addr.toString(), "192.168.1.1");
}

TEST(AddressV4Test, ArrayConstructor) {
    std::array<std::uint8_t, 4> octets{10, 0, 0, 1};
    AddressV4                   addr(octets);
    EXPECT_EQ(addr.toString(), "10.0.0.1");
}

TEST(AddressV4Test, FromStringValid) {
    auto result = AddressV4::fromString("192.168.1.1");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, AddressV4(192, 168, 1, 1));
}

TEST(AddressV4Test, FromStringBoundaryValues) {
    auto zero = AddressV4::fromString("0.0.0.0");
    ASSERT_TRUE(zero.has_value());
    EXPECT_EQ(*zero, AddressV4::any());

    auto max = AddressV4::fromString("255.255.255.255");
    ASSERT_TRUE(max.has_value());
    EXPECT_EQ(*max, AddressV4::broadcast());
}

TEST(AddressV4Test, FromStringLocalhost) {
    auto result = AddressV4::fromString("localhost");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, AddressV4::loopback());
}

TEST(AddressV4Test, FromStringAny) {
    auto result = AddressV4::fromString("any");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, AddressV4::any());
}

TEST(AddressV4Test, FromStringBroadcast) {
    auto result = AddressV4::fromString("broadcast");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, AddressV4::broadcast());
}

TEST(AddressV4Test, FromStringInvalidTooFewOctets) {
    auto result = AddressV4::fromString("192.168.1");
    EXPECT_FALSE(result.has_value());
}

TEST(AddressV4Test, FromStringInvalidOctetOutOfRange) {
    auto result = AddressV4::fromString("256.0.0.1");
    EXPECT_FALSE(result.has_value());
}

TEST(AddressV4Test, FromStringInvalidEmpty) {
    auto result = AddressV4::fromString("");
    EXPECT_FALSE(result.has_value());
}

TEST(AddressV4Test, FromStringInvalidGarbage) {
    auto result = AddressV4::fromString("not.an.ip.address");
    EXPECT_FALSE(result.has_value());
}

TEST(AddressV4Test, ToStringRoundTrip) {
    std::string_view input = "10.20.30.40";
    auto             addr  = AddressV4::fromString(input);
    ASSERT_TRUE(addr.has_value());
    EXPECT_EQ(addr->toString(), input);
}

TEST(AddressV4Test, SpecialAddresses) {
    EXPECT_EQ(AddressV4::any().toString(), "0.0.0.0");
    EXPECT_EQ(AddressV4::loopback().toString(), "127.0.0.1");
    EXPECT_EQ(AddressV4::broadcast().toString(), "255.255.255.255");
}

TEST(AddressV4Test, Equality) {
    EXPECT_EQ(AddressV4(1, 2, 3, 4), AddressV4(1, 2, 3, 4));
    EXPECT_NE(AddressV4(1, 2, 3, 4), AddressV4(4, 3, 2, 1));
}

// --- AddressV6 Tests ---

TEST(AddressV6Test, DefaultConstructor) {
    AddressV6 addr;
    EXPECT_EQ(addr, AddressV6::any());
}

TEST(AddressV6Test, FromStringFull) {
    auto result = AddressV6::fromString("2001:0db8:0000:0000:0000:0000:0000:0001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->toString(), "2001:db8::1");
}

TEST(AddressV6Test, FromStringCompressed) {
    auto result = AddressV6::fromString("2001:db8::1");
    ASSERT_TRUE(result.has_value());

    auto full = AddressV6::fromString("2001:0db8:0000:0000:0000:0000:0000:0001");
    ASSERT_TRUE(full.has_value());
    EXPECT_EQ(*result, *full);
}

TEST(AddressV6Test, FromStringLoopback) {
    auto result = AddressV6::fromString("::1");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, AddressV6::loopback());
}

TEST(AddressV6Test, FromStringAny) {
    auto result = AddressV6::fromString("::");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, AddressV6::any());
}

TEST(AddressV6Test, FromStringLocalhost) {
    auto result = AddressV6::fromString("localhost");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, AddressV6::loopback());
}

TEST(AddressV6Test, FromStringLeadingCompression) {
    auto result = AddressV6::fromString("::ffff:c0a8:1");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->toString(), "::ffff:c0a8:1");
}

TEST(AddressV6Test, FromStringTrailingCompression) {
    auto result = AddressV6::fromString("fe80::");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->toString(), "fe80::");
}

TEST(AddressV6Test, FromStringMiddleCompression) {
    auto result = AddressV6::fromString("fe80::1:2");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->toString(), "fe80::1:2");
}

TEST(AddressV6Test, FromStringInvalidEmpty) {
    auto result = AddressV6::fromString("");
    EXPECT_FALSE(result.has_value());
}

TEST(AddressV6Test, FromStringInvalidGarbage) {
    auto result = AddressV6::fromString("not:an:ipv6");
    EXPECT_FALSE(result.has_value());
}

TEST(AddressV6Test, FromStringInvalidTooManyGroups) {
    auto result = AddressV6::fromString("1:2:3:4:5:6:7:8:9");
    EXPECT_FALSE(result.has_value());
}

TEST(AddressV6Test, ToStringCompressesLongestZeroRun) {
    // 0:0:0:0:0:0:0:1 -> the 7 leading zeros should be compressed
    EXPECT_EQ(AddressV6::loopback().toString(), "::1");
    EXPECT_EQ(AddressV6::any().toString(), "::");
}

TEST(AddressV6Test, ToStringNoCompressionForSingleZeroGroup) {
    // RFC 5952 §4.2.2: :: must not be used for a single zero group
    auto result = AddressV6::fromString("2001:db8:0:1:1:1:1:1");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->toString(), "2001:db8:0:1:1:1:1:1");
}

TEST(AddressV6Test, ToStringRoundTrip) {
    std::string_view input = "fe80::1";
    auto             addr  = AddressV6::fromString(input);
    ASSERT_TRUE(addr.has_value());
    EXPECT_EQ(addr->toString(), input);
}

TEST(AddressV6Test, SpecialAddresses) {
    EXPECT_EQ(AddressV6::any().toString(), "::");
    EXPECT_EQ(AddressV6::loopback().toString(), "::1");
}

TEST(AddressV6Test, Equality) {
    EXPECT_EQ(AddressV6::loopback(), AddressV6::loopback());
    EXPECT_NE(AddressV6::any(), AddressV6::loopback());
}
