#pragma once

namespace mercury::socket::option {

struct ReuseAddress { bool value{true}; };
struct ReusePort    { bool value{true}; };
struct KeepAlive    { bool value{true}; };
struct NoDelay      { bool value{true}; };  // TCP_NODELAY

}    // namespace mercury::socket::option
