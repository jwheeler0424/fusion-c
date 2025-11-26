cmake_minimum_required(VERSION 3.14)
set(This Fsm)

set(Headers
    "include/fsm/fsm.hpp"
)

set(Sources
    "src/fsm.cpp"
    "include/fsm/fsm.hpp"
)

add_library(${This} STATIC ${Sources} ${Headers})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)

target_include_directories(${This} PUBLIC "include")

target_link_libraries(${This} PUBLIC
    Abnf
)

add_subdirectory(test)