enable_testing()
include(FetchGoogleTest)
include(GoogleTest)

add_executable(
    Tests
    TestLexer.cpp
)
target_link_libraries(
    Tests
    GTest::gtest_main
    Ratsh
)
gtest_discover_tests(Tests)