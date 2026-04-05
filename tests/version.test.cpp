#include <mercury/http/version.hpp>

#include <gtest/gtest.h>

namespace mercury::http::test {

// --- Valid versions ---

TEST(VersionFromStringTest, Http10) {
    auto result = version::fromString("HTTP/1.0");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Version::v1);
}

TEST(VersionFromStringTest, Http11) {
    auto result = version::fromString("HTTP/1.1");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Version::v1_1);
}

// --- Invalid inputs ---

TEST(VersionFromStringTest, EmptyReturnsNullopt) {
    EXPECT_FALSE(version::fromString("").has_value());
}

TEST(VersionFromStringTest, LowercaseReturnsNullopt) {
    EXPECT_FALSE(version::fromString("http/1.1").has_value());
}

TEST(VersionFromStringTest, Http20ReturnsNullopt) {
    EXPECT_FALSE(version::fromString("HTTP/2.0").has_value());
}

TEST(VersionFromStringTest, MissingPrefixReturnsNullopt) {
    EXPECT_FALSE(version::fromString("1.1").has_value());
}

TEST(VersionFromStringTest, PartialVersionReturnsNullopt) {
    EXPECT_FALSE(version::fromString("HTTP/1").has_value());
}

}    // namespace mercury::http::test
