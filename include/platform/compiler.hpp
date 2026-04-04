#pragma once

#include <cstdint>

#if defined(__clang__)
    #define PLATFORM_COMPILER_CLANG
#elif defined(__GNUC__)
    #define PLATFORM_COMPILER_GCC
#elif defined(_MSC_VER)
    #define PLATFORM_COMPILER_MSVC
#else
    #error "Unknown Compiler"
#endif

namespace platform {
enum class Compiler : std::uint8_t { Clang, Gcc, Msvc };

inline constexpr Compiler compiler =
#if defined(PLATFORM_COMPILER_CLANG)
    Compiler::Clang;
#elif defined(PLATFORM_COMPILER_GCC)
    Compiler::Gcc;
#elif defined(PLATFORM_COMPILER_MSVC)
    Compiler::Msvc;
#else
    #error "Unknown Compiler";
#endif

}    // namespace platform
