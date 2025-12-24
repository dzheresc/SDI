cmake_minimum_required(VERSION 3.10)
project(RateLimiter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable for the example/tests
add_executable(example
    example.cpp
    token_bucket.cpp
    leaking_bucket.cpp
    fixed_window.cpp
    sliding_window_log.cpp
    test_token_bucket.cpp
    test_leaking_bucket.cpp
    test_fixed_window.cpp
    test_sliding_window_log.cpp
)

# Create libraries for all rate limiters
add_library(token_bucket_lib
    token_bucket.cpp
)

add_library(leaking_bucket_lib
    leaking_bucket.cpp
)

add_library(fixed_window_lib
    fixed_window.cpp
)

add_library(sliding_window_log_lib
    sliding_window_log.cpp
)

target_include_directories(token_bucket_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(leaking_bucket_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(fixed_window_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(sliding_window_log_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

