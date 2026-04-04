cmake_minimum_required(VERSION 3.28)

project(
    mercury
    DESCRIPTION "An HTTP server written in C++, focusing on compile time safety & guarantees"
    LANGUAGES CXX
    VERSION 0.1.0
)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(
    MERCURY_BUILD_TESTS
    "Enable building tests and infrastructure. Default: ${PROJECT_IS_TOP_LEVEL}. Values { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

add_subdirectory(src)

if(MERCURY_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()


