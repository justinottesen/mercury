#pragma once

#include <platform/platform.hpp>

#if defined(PLATFORM_OS_POSIX)
    #include <mercury/socket/drivers/posix_socket.hpp>
#else
    #error "Unsupported OS"
#endif

namespace mercury::socket::detail {

using Driver =
#if defined(PLATFORM_OS_POSIX)
    PosixDriver;
#else
    #error "Unsupported OS"
#endif

}    // namespace mercury::socket::detail
