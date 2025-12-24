cmake_minimum_required(VERSION 3.10)
project(TokenBucketRateLimiter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable for the example
add_executable(example
    example.cpp
    token_bucket.cpp
)

# Optional: Create a library version
add_library(token_bucket_lib
    token_bucket.cpp
)

target_include_directories(token_bucket_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

