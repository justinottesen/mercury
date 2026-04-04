#pragma once

#if defined(__has_include) && __has_include(<unistd.h>)
    #include <unistd.h>
#endif

#if defined(_POSIX_VERSION)
    #define PLATFORM_OS_POSIX
#elif defined(_WIN32)
    #define PLATFORM_OS_WIN32
#else
    #error "Unknown OS"
#endif

namespace platform {
enum class Os : bool { Posix, Win32 };

inline constexpr Os os =
#if defined(PLATFORM_OS_POSIX)
    Os::Posix;
#elif defined(PLATFORM_OS_WIN32)
    Os::Win32;
#else
    #error "Unknown OS";
#endif

}    // namespace platform
