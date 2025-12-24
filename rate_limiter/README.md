# Token Bucket Rate Limiter

A C++ implementation of a rate limiter using the Token Bucket algorithm.

## Overview

The Token Bucket algorithm is a popular rate limiting technique that:
- Maintains a bucket with a maximum capacity of tokens
- Refills tokens at a constant rate (tokens per second)
- Allows requests when tokens are available
- Denies requests when the bucket is empty

## Features

- **Thread-safe**: Uses mutex for concurrent access
- **Precise timing**: Uses `std::chrono` for accurate time tracking
- **Flexible**: Supports consuming single or multiple tokens
- **Efficient**: O(1) time complexity for token consumption

## Building

### Using CMake

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Manual Compilation

```bash
g++ -std=c++17 -pthread example.cpp token_bucket.cpp -o example
```

## Usage

```cpp
#include "token_bucket.h"

// Create a rate limiter: 10 tokens capacity, 2 tokens/second refill rate
TokenBucket limiter(10.0, 2.0);

// Try to consume a token
if (limiter.tryConsume()) {
    // Request allowed
} else {
    // Rate limited
}

// Try to consume multiple tokens
if (limiter.tryConsume(5)) {
    // 5 tokens consumed
}

// Check available tokens
double available = limiter.getAvailableTokens();
```

## Running the Example

```bash
./example
```

The example demonstrates:
- Basic usage and token consumption
- Token refill over time
- Burst capacity handling
- Thread-safe concurrent access

## API Reference

### Constructor
```cpp
TokenBucket(double capacity, double refillRate)
```
- `capacity`: Maximum number of tokens the bucket can hold
- `refillRate`: Number of tokens added per second

### Methods
- `bool tryConsume()`: Try to consume one token
- `bool tryConsume(int tokens)`: Try to consume multiple tokens
- `double getAvailableTokens()`: Get current available tokens
- `double getCapacity()`: Get bucket capacity
- `double getRefillRate()`: Get refill rate
- `void reset()`: Reset bucket to full capacity

## Algorithm Details

The token bucket algorithm works as follows:

1. **Initialization**: Bucket starts with full capacity
2. **Refill**: Tokens are continuously added at the refill rate
3. **Consumption**: Each request consumes one or more tokens
4. **Rate Limiting**: Requests are denied when insufficient tokens are available

The implementation uses nanosecond precision for accurate token refill calculations.
