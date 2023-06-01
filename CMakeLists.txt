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

include(GoogleTest)

add_executable(ratsh
    main.cpp
    AST.h
    Shell.cpp
    Shell.h
    Lexer.cpp
    Lexer.h
    Parser.h
    Parser.cpp
)

target_link_libraries(ratsh gtest_main)

