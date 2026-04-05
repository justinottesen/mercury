#include <mercury/net/slice.hpp>

#include <gtest/gtest.h>

namespace mercury::net::test {

using namespace std::string_view_literals;

// --- Equality ---

TEST(SliceTest, EqualsSameContent) {
    Slice const s{"hello"sv};
    EXPECT_EQ(s, "hello");
}

TEST(SliceTest, EqualsDifferentContent) {
    Slice const s{"hello"sv};
    EXPECT_NE(s, "world");
}

TEST(SliceTest, EqualsEmpty) {
    Slice const s;
    EXPECT_EQ(s, "");
}

// --- iequals ---

TEST(SliceTest, IEqualsExactMatch) {
    Slice const s{"Content-Type"sv};
    EXPECT_TRUE(iequals(s, "Content-Type"));
}

TEST(SliceTest, IEqualsLowercase) {
    Slice const s{"Content-Type"sv};
    EXPECT_TRUE(iequals(s, "content-type"));
}

TEST(SliceTest, IEqualsUppercase) {
    Slice const s{"Content-Type"sv};
    EXPECT_TRUE(iequals(s, "CONTENT-TYPE"));
}

TEST(SliceTest, IEqualsDifferentValue) {
    Slice const s{"Content-Type"sv};
    EXPECT_FALSE(iequals(s, "content-length"));
}

TEST(SliceTest, IEqualsDifferentLength) {
    Slice const s{"host"sv};
    EXPECT_FALSE(iequals(s, "hostname"));
}

TEST(SliceTest, IEqualsEmpty) {
    Slice const s;
    EXPECT_TRUE(iequals(s, ""));
    EXPECT_FALSE(iequals(s, "host"));
}

// --- to_string ---

TEST(SliceTest, ToStringCopiesContent) {
    Slice const s{"hello"sv};
    EXPECT_EQ(s.to_string(), "hello");
}

TEST(SliceTest, ToStringEmpty) {
    Slice const s;
    EXPECT_EQ(s.to_string(), "");
}

}    // namespace mercury::net::test
