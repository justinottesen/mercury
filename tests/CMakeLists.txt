find_package(GTest REQUIRED)

add_executable(address.test address.test.cpp)
target_link_libraries(address.test PRIVATE
    mercury::headers
    mercury::warnings
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(address.test)

add_executable(endpoint.test endpoint.test.cpp)
target_link_libraries(endpoint.test PRIVATE
    mercury::headers
    mercury::warnings
    GTest::gtest_main
)

gtest_discover_tests(endpoint.test)

add_executable(socket.test socket.test.cpp)
target_link_libraries(socket.test PRIVATE
    mercury::headers
    mercury::warnings
    GTest::gtest_main
)

gtest_discover_tests(socket.test)

add_executable(read_buffer.test read_buffer.test.cpp)
target_link_libraries(read_buffer.test PRIVATE
    mercury::headers
    mercury::warnings
    GTest::gtest_main
)

gtest_discover_tests(read_buffer.test)

add_executable(http_parse.test http_parse.test.cpp)
target_link_libraries(http_parse.test PRIVATE
    mercury::headers
    mercury::warnings
    GTest::gtest_main
)

gtest_discover_tests(http_parse.test)
