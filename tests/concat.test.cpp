#include <future/concat.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

namespace future::views::test {

namespace {

template <std::ranges::input_range View>
auto collect(View const& v) -> std::vector<std::ranges::range_value_t<View>> {
    std::vector<std::ranges::range_value_t<View>> result;
    std::ranges::copy(v, std::back_inserter(result));
    return result;
}

}    // namespace

TEST(ConcatTest, TwoNonEmptyRanges) {
    std::vector<int> const a{1, 2, 3};
    std::vector<int> const b{4, 5, 6};
    EXPECT_EQ(collect(future::views::concat(a, b)), (std::vector<int>{1, 2, 3, 4, 5, 6}));
}

TEST(ConcatTest, ThreeRanges) {
    std::vector<int> const a{1};
    std::vector<int> const b{2, 3};
    std::vector<int> const c{4};
    EXPECT_EQ(collect(future::views::concat(a, b, c)), (std::vector<int>{1, 2, 3, 4}));
}

TEST(ConcatTest, EmptyFirstRange) {
    std::vector<int> const a{};
    std::vector<int> const b{1, 2, 3};
    EXPECT_EQ(collect(future::views::concat(a, b)), (std::vector<int>{1, 2, 3}));
}

TEST(ConcatTest, EmptyMiddleRange) {
    std::vector<int> const a{1};
    std::vector<int> const b{};
    std::vector<int> const c{2};
    EXPECT_EQ(collect(future::views::concat(a, b, c)), (std::vector<int>{1, 2}));
}

TEST(ConcatTest, EmptyLastRange) {
    std::vector<int> const a{1, 2};
    std::vector<int> const b{};
    EXPECT_EQ(collect(future::views::concat(a, b)), (std::vector<int>{1, 2}));
}

TEST(ConcatTest, AllEmptyRanges) {
    std::vector<int> const a{};
    std::vector<int> const b{};
    EXPECT_TRUE(collect(future::views::concat(a, b)).empty());
}

TEST(ConcatTest, SingleElementRanges) {
    std::vector<int> const a{42};
    std::vector<int> const b{7};
    EXPECT_EQ(collect(future::views::concat(a, b)), (std::vector<int>{42, 7}));
}

}    // namespace future::views::test
