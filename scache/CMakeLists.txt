cmake_minimum_required(VERSION 3.10)
project(StringsCache)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add comprehensive test executable
add_executable(example_comprehensive
    example_comprehensive.cpp
    test_strings_cache_comprehensive.cpp
)

