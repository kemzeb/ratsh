add_library(Ratsh
    AST.h
    AST.cpp
    Builtins.h
    Builtins.cpp
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

add_executable(Main main.cpp)
target_link_libraries(Main PRIVATE Ratsh)

target_include_directories(Ratsh PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})