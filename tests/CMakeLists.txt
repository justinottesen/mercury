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

add_executable(slice.test slice.test.cpp)
target_link_libraries(slice.test PRIVATE
    mercury::headers
    mercury::warnings
    GTest::gtest_main
)

gtest_discover_tests(slice.test)

add_executable(method.test method.test.cpp)
target_link_libraries(method.test PRIVATE
    mercury::headers
    mercury::warnings
    GTest::gtest_main
)

gtest_discover_tests(method.test)

add_executable(version.test version.test.cpp)
target_link_libraries(version.test PRIVATE
    mercury::headers
    mercury::warnings
    GTest::gtest_main
)

gtest_discover_tests(version.test)

add_executable(concat.test concat.test.cpp)
target_link_libraries(concat.test PRIVATE
    mercury::headers
    mercury::warnings
    GTest::gtest_main
)

gtest_discover_tests(concat.test)
