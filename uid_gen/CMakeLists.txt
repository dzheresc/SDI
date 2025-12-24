cmake_minimum_required(VERSION 3.10)
project(SnowflakeIdGenerator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable for the example/tests
add_executable(example
    example.cpp
    snowflake_id.cpp
    test_snowflake.cpp
)

# Create library
add_library(snowflake_id_lib
    snowflake_id.cpp
)

target_include_directories(snowflake_id_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

