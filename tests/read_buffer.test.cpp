#include <mercury/net/read_buffer.hpp>

#include <gtest/gtest.h>

namespace mercury::net::test {

namespace {

// Feeds fixed data one chunk at a time, simulating partial reads.
struct FakeSocket {
    explicit FakeSocket(std::vector<std::vector<std::byte>> chunks)
        : m_chunks{std::move(chunks)} {}

    [[nodiscard]] auto recv(std::span<std::byte> buf) const
        -> std::expected<std::size_t, std::errc> {
        if (m_pos >= m_chunks.size()) { return std::unexpected{std::errc::connection_reset}; }
        auto const& chunk = m_chunks[m_pos++];
        auto const  n     = std::min(chunk.size(), buf.size());
        std::ranges::copy(std::span{chunk}.subspan(0, n), buf.begin());
        return n;
    }

private:
    std::vector<std::vector<std::byte>> m_chunks;
    mutable std::size_t                 m_pos{0};
};

auto toBytes(std::string_view s) -> std::vector<std::byte> {
    return s | std::views::transform([](char c) -> std::byte { return static_cast<std::byte>(c); })
         | std::ranges::to<std::vector<std::byte>>();
}

}    // namespace

// --- readUntil ---

TEST(ReadBufferReadUntilTest, SingleChunkDelimiterFound) {
    ReadBuffer buf{FakeSocket{{toBytes("hello\r\nworld")}}};
    auto       result = buf.readUntil("\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "hello");
}

TEST(ReadBufferReadUntilTest, DelimiterSpansChunks) {
    ReadBuffer buf{FakeSocket{{toBytes("hel\r"), toBytes("\nworld")}}};
    auto       result = buf.readUntil("\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "hel");
}

TEST(ReadBufferReadUntilTest, DelimiterAtStart) {
    ReadBuffer buf{FakeSocket{{toBytes("\r\ndata")}}};
    auto       result = buf.readUntil("\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "");
}

TEST(ReadBufferReadUntilTest, MultipleConsecutiveReads) {
    ReadBuffer buf{FakeSocket{{toBytes("line1\r\nline2\r\n")}}};
    auto       first = buf.readUntil("\r\n");
    ASSERT_TRUE(first.has_value());
    EXPECT_EQ(*first, "line1");
    auto second = buf.readUntil("\r\n");
    ASSERT_TRUE(second.has_value());
    EXPECT_EQ(*second, "line2");
}

TEST(ReadBufferReadUntilTest, ErrorWhenNoDelimiterAndSocketDone) {
    ReadBuffer buf{FakeSocket{{toBytes("no delimiter here")}}};
    auto       result = buf.readUntil("\r\n");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::errc::connection_reset);
}

// --- readExact ---

TEST(ReadBufferReadExactTest, ExactlyOneChunk) {
    ReadBuffer buf{FakeSocket{{toBytes("hello")}}};
    auto       result = buf.readExact(5);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, toBytes("hello"));
}

TEST(ReadBufferReadExactTest, SpansMultipleChunks) {
    ReadBuffer buf{FakeSocket{{toBytes("hel"), toBytes("lo")}}};
    auto       result = buf.readExact(5);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, toBytes("hello"));
}

TEST(ReadBufferReadExactTest, LessThanOneChunk) {
    ReadBuffer buf{FakeSocket{{toBytes("hello world")}}};
    auto       result = buf.readExact(5);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, toBytes("hello"));
}

TEST(ReadBufferReadExactTest, ConsecutiveReads) {
    ReadBuffer buf{FakeSocket{{toBytes("helloworld")}}};
    auto       first = buf.readExact(5);
    ASSERT_TRUE(first.has_value());
    EXPECT_EQ(*first, toBytes("hello"));
    auto second = buf.readExact(5);
    ASSERT_TRUE(second.has_value());
    EXPECT_EQ(*second, toBytes("world"));
}

TEST(ReadBufferReadExactTest, ErrorWhenSocketExhausted) {
    ReadBuffer buf{FakeSocket{{toBytes("hi")}}};
    auto       result = buf.readExact(10);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::errc::connection_reset);
}

// --- mixed ---

TEST(ReadBufferMixedTest, ReadUntilThenReadExact) {
    ReadBuffer buf{FakeSocket{{toBytes("header\r\nbody!")}}};
    auto       header = buf.readUntil("\r\n");
    ASSERT_TRUE(header.has_value());
    EXPECT_EQ(*header, "header");
    auto body = buf.readExact(5);
    ASSERT_TRUE(body.has_value());
    EXPECT_EQ(*body, toBytes("body!"));
}

}    // namespace mercury::net::test
