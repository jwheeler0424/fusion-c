cmake_minimum_required(VERSION 3.14)
set(This AbnfTests)

set(Sources
    src/abnf.test.cpp
)

add_executable(${This} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Tests
)

target_include_directories(${This} PRIVATE ..)

target_link_libraries(${This} PUBLIC
    gtest_main
    Fsm
)

add_test(
    NAME ${This}
    COMMAND ${This}
)