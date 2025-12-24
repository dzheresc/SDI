cmake_minimum_required(VERSION 3.10)
project(ConsistentHashing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable for the example/tests
add_executable(example
    example.cpp
    consistent_hash.cpp
    test_consistent_hash.cpp
)

# Create library
add_library(consistent_hash_lib
    consistent_hash.cpp
)

target_include_directories(consistent_hash_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

