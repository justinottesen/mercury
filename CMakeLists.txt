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

add_library(mercury.headers INTERFACE)
target_include_directories(mercury.headers INTERFACE include)
add_library(mercury::headers ALIAS mercury.headers)

add_subdirectory(src)

if(MERCURY_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()


