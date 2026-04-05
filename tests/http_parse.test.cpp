#include <gtest/gtest.h>
#include <mercury/http/parse.hpp>

namespace mercury::http::test {

namespace {

struct FakeSocket {
    explicit FakeSocket(std::string data)
        : m_data{std::move(data)} {}

    [[nodiscard]] auto recv(std::span<std::byte> buf) const
        -> std::expected<std::size_t, std::errc> {
        if (m_pos >= m_data.size()) { return std::unexpected{std::errc::connection_reset}; }
        auto const n = std::min(buf.size(), m_data.size() - m_pos);
        std::ranges::copy(std::span{reinterpret_cast<const std::byte*>(m_data.data() + m_pos),
                                    n},    // NOLINT(*-reinterpret-cast)
                          buf.begin());
        m_pos += n;
        return n;
    }

private:
    std::string         m_data;
    mutable std::size_t m_pos{0};
};

auto makeBuffer(std::string data) { return net::ReadBuffer{FakeSocket{std::move(data)}}; }

}    // namespace

// --- Method parsing ---

TEST(HttpParseTest, GetRequest) {
    auto buf    = makeBuffer("GET / HTTP/1.1\r\nHost: example.com\r\n\r\n");
    auto result = parseRequest(buf);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->method, Method::Get);
}

TEST(HttpParseTest, PostRequest) {
    auto buf    = makeBuffer("POST /submit HTTP/1.1\r\nHost: example.com\r\n\r\n");
    auto result = parseRequest(buf);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->method, Method::Post);
}

TEST(HttpParseTest, UnknownMethodReturnsError) {
    auto buf    = makeBuffer("FOO / HTTP/1.1\r\n\r\n");
    auto result = parseRequest(buf);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::InvalidMethod);
}

// --- Target parsing ---

TEST(HttpParseTest, RootTarget) {
    auto buf    = makeBuffer("GET / HTTP/1.1\r\n\r\n");
    auto result = parseRequest(buf);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->target, "/");
}

TEST(HttpParseTest, PathWithQuery) {
    auto buf    = makeBuffer("GET /search?q=foo HTTP/1.1\r\n\r\n");
    auto result = parseRequest(buf);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->target, "/search?q=foo");
}

// --- Version parsing ---

TEST(HttpParseTest, Http11) {
    auto buf    = makeBuffer("GET / HTTP/1.1\r\n\r\n");
    auto result = parseRequest(buf);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->version, Version::v1_1);
}

TEST(HttpParseTest, Http10) {
    auto buf    = makeBuffer("GET / HTTP/1.0\r\n\r\n");
    auto result = parseRequest(buf);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->version, Version::v1);
}

TEST(HttpParseTest, UnknownVersionReturnsError) {
    auto buf    = makeBuffer("GET / HTTP/2.0\r\n\r\n");
    auto result = parseRequest(buf);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::InvalidVersion);
}

// --- Header parsing ---

TEST(HttpParseTest, SingleHeader) {
    auto buf    = makeBuffer("GET / HTTP/1.1\r\nHost: example.com\r\n\r\n");
    auto result = parseRequest(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->headers.size(), 1U);
    EXPECT_EQ(result->headers[0].first, "Host");
    EXPECT_EQ(result->headers[0].second, "example.com");
}

TEST(HttpParseTest, MultipleHeaders) {
    auto buf =
        makeBuffer("GET / HTTP/1.1\r\nHost: example.com\r\nContent-Type: text/plain\r\n\r\n");
    auto result = parseRequest(buf);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->headers.size(), 2U);
    EXPECT_EQ(result->headers[0].first, "Host");
    EXPECT_EQ(result->headers[1].first, "Content-Type");
    EXPECT_EQ(result->headers[1].second, "text/plain");
}

TEST(HttpParseTest, NoHeaders) {
    auto buf    = makeBuffer("GET / HTTP/1.1\r\n\r\n");
    auto result = parseRequest(buf);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->headers.empty());
}

TEST(HttpParseTest, MalformedHeaderReturnsError) {
    auto buf    = makeBuffer("GET / HTTP/1.1\r\nBadHeader\r\n\r\n");
    auto result = parseRequest(buf);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::InvalidHeader);
}

// --- Request line errors ---

TEST(HttpParseTest, MissingVersionReturnsError) {
    auto buf    = makeBuffer("GET /\r\n\r\n");
    auto result = parseRequest(buf);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::InvalidRequestLine);
}

TEST(HttpParseTest, EmptyRequestLineReturnsError) {
    auto buf    = makeBuffer("\r\n\r\n");
    auto result = parseRequest(buf);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::InvalidRequestLine);
}

// --- IO errors ---

TEST(HttpParseTest, IoErrorPropagated) {
    auto buf    = makeBuffer("");    // socket immediately done
    auto result = parseRequest(buf);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::IoError);
}

}    // namespace mercury::http::test
