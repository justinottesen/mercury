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

// Holds the arena alongside the parse result so slices remain valid
// for the duration of each test.
struct TestContext {
    std::array<std::byte, 4096>         arenaBuf{};
    std::pmr::monotonic_buffer_resource arena{arenaBuf.data(), arenaBuf.size()};

    [[nodiscard]] auto parse(std::string data) {
        auto buf = net::ReadBuffer{FakeSocket{std::move(data)}};
        return parseRequest(buf, arena);
    }
};

}    // namespace

// --- Method parsing ---

TEST(HttpParseTest, GetRequest) {
    TestContext ctx;
    auto result = ctx.parse("GET / HTTP/1.1\r\nHost: example.com\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->method, Method::Get);
}

TEST(HttpParseTest, PostRequest) {
    TestContext ctx;
    auto result = ctx.parse("POST /submit HTTP/1.1\r\nHost: example.com\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->method, Method::Post);
}

TEST(HttpParseTest, UnknownMethodReturnsError) {
    TestContext ctx;
    auto result = ctx.parse("FOO / HTTP/1.1\r\n\r\n");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::InvalidMethod);
}

// --- Target parsing ---

TEST(HttpParseTest, RootTarget) {
    TestContext ctx;
    auto result = ctx.parse("GET / HTTP/1.1\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->target, "/");
}

TEST(HttpParseTest, PathWithQuery) {
    TestContext ctx;
    auto result = ctx.parse("GET /search?q=foo HTTP/1.1\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->target, "/search?q=foo");
}

// --- Version parsing ---

TEST(HttpParseTest, Http11) {
    TestContext ctx;
    auto result = ctx.parse("GET / HTTP/1.1\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->version, Version::v1_1);
}

TEST(HttpParseTest, Http10) {
    TestContext ctx;
    auto result = ctx.parse("GET / HTTP/1.0\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->version, Version::v1);
}

TEST(HttpParseTest, UnknownVersionReturnsError) {
    TestContext ctx;
    auto result = ctx.parse("GET / HTTP/2.0\r\n\r\n");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::InvalidVersion);
}

// --- Header parsing ---

TEST(HttpParseTest, SingleHeader) {
    TestContext ctx;
    auto result = ctx.parse("GET / HTTP/1.1\r\nHost: example.com\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->headers.size(), 1U);
    EXPECT_EQ(result->headers[0].first, "Host");
    EXPECT_EQ(result->headers[0].second, "example.com");
}

TEST(HttpParseTest, MultipleHeaders) {
    TestContext ctx;
    auto result = ctx.parse("GET / HTTP/1.1\r\nHost: example.com\r\nContent-Type: text/plain\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->headers.size(), 2U);
    EXPECT_EQ(result->headers[0].first, "Host");
    EXPECT_EQ(result->headers[1].first, "Content-Type");
    EXPECT_EQ(result->headers[1].second, "text/plain");
}

TEST(HttpParseTest, NoHeaders) {
    TestContext ctx;
    auto result = ctx.parse("GET / HTTP/1.1\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->headers.empty());
}

TEST(HttpParseTest, MalformedHeaderReturnsError) {
    TestContext ctx;
    auto result = ctx.parse("GET / HTTP/1.1\r\nBadHeader\r\n\r\n");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::InvalidHeader);
}

// --- Request line errors ---

TEST(HttpParseTest, MissingVersionReturnsError) {
    TestContext ctx;
    auto result = ctx.parse("GET /\r\n\r\n");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::InvalidRequestLine);
}

TEST(HttpParseTest, EmptyRequestLineReturnsError) {
    TestContext ctx;
    auto result = ctx.parse("\r\n\r\n");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::InvalidRequestLine);
}

// --- findHeader ---

TEST(HttpParseTest, FindHeaderExactCase) {
    TestContext ctx;
    auto result = ctx.parse("GET / HTTP/1.1\r\nHost: example.com\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    auto host = findHeader(*result, "Host");
    ASSERT_TRUE(host.has_value());
    EXPECT_EQ(*host, "example.com");
}

TEST(HttpParseTest, FindHeaderCaseInsensitive) {
    TestContext ctx;
    auto result = ctx.parse("GET / HTTP/1.1\r\nContent-Type: text/plain\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(findHeader(*result, "content-type").has_value());
    EXPECT_TRUE(findHeader(*result, "CONTENT-TYPE").has_value());
}

TEST(HttpParseTest, FindHeaderMissing) {
    TestContext ctx;
    auto result = ctx.parse("GET / HTTP/1.1\r\nHost: example.com\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(findHeader(*result, "Authorization").has_value());
}

// --- Remaining methods ---

TEST(HttpParseTest, HeadRequest) {
    TestContext ctx;
    auto        result = ctx.parse("HEAD / HTTP/1.1\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->method, Method::Head);
}

TEST(HttpParseTest, PutRequest) {
    TestContext ctx;
    auto        result = ctx.parse("PUT /resource HTTP/1.1\r\nHost: example.com\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->method, Method::Put);
}

TEST(HttpParseTest, DeleteRequest) {
    TestContext ctx;
    auto        result = ctx.parse("DELETE /resource HTTP/1.1\r\nHost: example.com\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->method, Method::Delete);
}

TEST(HttpParseTest, OptionsRequest) {
    TestContext ctx;
    auto        result = ctx.parse("OPTIONS * HTTP/1.1\r\nHost: example.com\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->method, Method::Options);
}

TEST(HttpParseTest, PatchRequest) {
    TestContext ctx;
    auto        result = ctx.parse("PATCH /resource HTTP/1.1\r\nHost: example.com\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->method, Method::Patch);
}

// --- Header value edge cases ---

TEST(HttpParseTest, HeaderValueContainingColon) {
    TestContext ctx;
    auto        result = ctx.parse("GET / HTTP/1.1\r\nHost: example.com:8080\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    auto host = findHeader(*result, "Host");
    ASSERT_TRUE(host.has_value());
    EXPECT_EQ(*host, "example.com:8080");
}

TEST(HttpParseTest, DuplicateHeadersBothPresent) {
    TestContext ctx;
    auto result = ctx.parse(
        "GET / HTTP/1.1\r\nAccept: text/html\r\nAccept: application/json\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->headers.size(), 2U);
    EXPECT_EQ(result->headers[0].second, "text/html");
    EXPECT_EQ(result->headers[1].second, "application/json");
}

// --- IO errors ---

TEST(HttpParseTest, IoErrorPropagated) {
    TestContext ctx;
    auto result = ctx.parse("");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::IoError);
}

}    // namespace mercury::http::test
