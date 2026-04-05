#include <mercury/http/method.hpp>

#include <gtest/gtest.h>

namespace mercury::http::test {

// --- Valid methods ---

TEST(MethodFromStringTest, Get) {
    auto result = method::fromString("GET");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Method::Get);
}

TEST(MethodFromStringTest, Head) {
    auto result = method::fromString("HEAD");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Method::Head);
}

TEST(MethodFromStringTest, Post) {
    auto result = method::fromString("POST");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Method::Post);
}

TEST(MethodFromStringTest, Put) {
    auto result = method::fromString("PUT");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Method::Put);
}

TEST(MethodFromStringTest, Delete) {
    auto result = method::fromString("DELETE");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Method::Delete);
}

TEST(MethodFromStringTest, Options) {
    auto result = method::fromString("OPTIONS");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Method::Options);
}

TEST(MethodFromStringTest, Patch) {
    auto result = method::fromString("PATCH");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Method::Patch);
}

// --- Invalid inputs ---

TEST(MethodFromStringTest, EmptyReturnsNullopt) {
    EXPECT_FALSE(method::fromString("").has_value());
}

TEST(MethodFromStringTest, LowercaseReturnsNullopt) {
    EXPECT_FALSE(method::fromString("get").has_value());
    EXPECT_FALSE(method::fromString("post").has_value());
}

TEST(MethodFromStringTest, PrefixMatchReturnsNullopt) {
    EXPECT_FALSE(method::fromString("GETTER").has_value());
    EXPECT_FALSE(method::fromString("POSTING").has_value());
}

TEST(MethodFromStringTest, SuffixMatchReturnsNullopt) {
    EXPECT_FALSE(method::fromString("XGET").has_value());
}

}    // namespace mercury::http::test
