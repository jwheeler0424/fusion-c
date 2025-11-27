cmake_minimum_required(VERSION 3.14)
set(This FsmTests)

set(Sources
    src/fsm.test.cpp
    src/streaming.test.cpp
    src/backtracking.test.cpp
    src/actions.test.cpp
)

add_executable(${This} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Tests
)

target_include_directories(${This} PRIVATE ..)

target_link_libraries(${This} PUBLIC
    gtest_main
    Fsm
    Abnf
)

add_test(
    NAME ${This}
    COMMAND ${This}
)