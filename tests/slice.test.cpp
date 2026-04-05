#include <mercury/net/slice.hpp>

#include <gtest/gtest.h>

namespace mercury::net::test {

using namespace std::string_view_literals;

// --- Constructors and accessors ---

TEST(SliceTest, PtrSizeConstructor) {
    static constexpr std::string_view kData = "hello";
    Slice const                       s{kData.data(), kData.size()};
    EXPECT_EQ(s, "hello");
}

TEST(SliceTest, DataPointsToUnderlyingBytes) {
    static constexpr std::string_view kData = "hello";
    Slice const                       s{kData.data(), kData.size()};
    EXPECT_EQ(s.data(), kData.data());
    EXPECT_EQ(s.size(), kData.size());
}

TEST(SliceTest, EmptyReturnsTrueForDefault) {
    Slice const s;
    EXPECT_TRUE(s.empty());
}

TEST(SliceTest, EmptyReturnsFalseForContent) {
    Slice const s{"hello"sv};
    EXPECT_FALSE(s.empty());
}

TEST(SliceTest, IteratorYieldsContent) {
    Slice const s{"abc"sv};
    EXPECT_TRUE(std::ranges::equal(s, "abc"sv));
}

// --- Slice == Slice ---

TEST(SliceTest, SliceEqualsSliceSameContent) {
    Slice const a{"hello"sv};
    Slice const b{"hello"sv};
    EXPECT_EQ(a, b);
}

TEST(SliceTest, SliceEqualsSliceDifferentContent) {
    Slice const a{"hello"sv};
    Slice const b{"world"sv};
    EXPECT_NE(a, b);
}

// --- ostream ---

TEST(SliceTest, OstreamOutputsContent) {
    Slice const       s{"hello"sv};
    std::ostringstream oss;
    oss << s;
    EXPECT_EQ(oss.str(), "hello");
}

TEST(SliceTest, OstreamOutputsEmpty) {
    Slice const        s;
    std::ostringstream oss;
    oss << s;
    EXPECT_EQ(oss.str(), "");
}

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
