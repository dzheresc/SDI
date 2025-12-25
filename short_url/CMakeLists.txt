cmake_minimum_required(VERSION 3.10)
project(UrlShortener)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the key_value_store directory to include path
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../key_value_store)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../consistent_hashing)

# Add executable for the example/tests (unordered_map version)
add_executable(example
    example.cpp
    url_shortener.cpp
    test_url_shortener.cpp
)

# Add executable for KeyValue store version
add_executable(example_kv
    example_kv.cpp
    url_shortener_kv.cpp
    test_url_shortener_kv.cpp
    ../key_value_store/kv_store.cpp
    ../consistent_hashing/consistent_hash.cpp
)

# Create libraries
add_library(url_shortener_lib
    url_shortener.cpp
)

add_library(url_shortener_kv_lib
    url_shortener_kv.cpp
    ../key_value_store/kv_store.cpp
    ../consistent_hashing/consistent_hash.cpp
)

target_include_directories(url_shortener_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(url_shortener_kv_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../key_value_store
    ${CMAKE_CURRENT_SOURCE_DIR}/../consistent_hashing
)

