cmake_minimum_required(VERSION 3.14)
set(This Uri)

set(Headers
    "include/uri/uri.hpp"
)

set(Sources
    "src/uri.cpp"
    "include/uri/uri.hpp"
)

add_library(${This} STATIC ${Sources} ${Headers})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)

target_include_directories(${This} PUBLIC "include")

target_link_libraries(${This} PUBLIC
    Abnf
    Fsm
)

add_subdirectory(test)