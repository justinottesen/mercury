find_package(GTest REQUIRED)

add_executable(address.test address.test.cpp)
target_link_libraries(address.test PRIVATE
    mercury::headers
    mercury::warnings
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(address.test)
