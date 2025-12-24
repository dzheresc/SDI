cmake_minimum_required(VERSION 3.10)
project(KeyValueStore)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the consistent_hashing directory to include path
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../consistent_hashing)

# Add executable for the example/tests
add_executable(example
    example.cpp
    kv_store.cpp
    test_kv_store.cpp
    ../consistent_hashing/consistent_hash.cpp
)

# Create library
add_library(kv_store_lib
    kv_store.cpp
    ../consistent_hashing/consistent_hash.cpp
)

target_include_directories(kv_store_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../consistent_hashing
)

