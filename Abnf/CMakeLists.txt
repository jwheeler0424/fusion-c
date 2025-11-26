cmake_minimum_required(VERSION 3.14)
set(This Abnf)

set(Headers
    "include/abnf/abnf.hpp"
)

set(Sources
    "src/abnf.cpp"
    "include/abnf/abnf.hpp"
)

add_library(${This} STATIC ${Sources} ${Headers})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)

target_include_directories(${This} PUBLIC "include")

add_subdirectory("test")