cmake_minimum_required(VERSION 3.22)

project("ratsh" CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Wno-unknown-warning-option)
add_compile_options(-Wno-unused-command-line-argument)
add_compile_options(-fdiagnostics-color=always)
add_compile_options(-fno-exceptions)

# To generate compile_commands.json for clangd.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

enable_testing()
include(FetchGoogleTest)
include(GoogleTest)

set(sources
    AST.h
    AST.cpp
    FileDescription.h
    FileDescription.cpp
    Lexer.cpp
    Lexer.h
    Parser.h
    Parser.cpp
    Shell.cpp
    Shell.h
    Value.h
)
add_library(ratsh STATIC ${sources})
add_executable(main main.cpp)
target_link_libraries(main ratsh)

add_executable(
    tests
    Tests/TestLexer.cpp
)
target_link_libraries(
    tests
    GTest::gtest_main
    ratsh
)
gtest_discover_tests(tests)