find_package(GTest REQUIRED)

add_executable(fastlex_ascii_test ascii_test.cpp)
target_link_libraries(fastlex_ascii_test PRIVATE fastlex::fastlex GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(fastlex_ascii_test)
