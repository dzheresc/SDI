# Rate Limiter Library

A comprehensive C++ implementation of multiple rate limiting algorithms, each with different characteristics and use cases.

## Overview

This library provides five different rate limiting algorithms:

1. **Token Bucket** - Allows bursts, smooth refill
2. **Leaking Bucket** - Smooth output rate, queue-based
3. **Fixed Window** - Simple, resets at fixed intervals
4. **Sliding Window Log** - Accurate, stores all timestamps
5. **Sliding Window Counter** - Memory efficient, weighted approximation

## Features

- **Thread-safe**: All implementations use mutex for concurrent access
- **Precise timing**: Uses `std::chrono` for nanosecond-accurate time tracking
- **Comprehensive testing**: Each algorithm includes extensive test suites
- **Well-documented**: Clear API and usage examples

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
g++ -std=c++17 -pthread example.cpp token_bucket.cpp leaking_bucket.cpp fixed_window.cpp sliding_window_log.cpp sliding_window_counter.cpp test_token_bucket.cpp test_leaking_bucket.cpp test_fixed_window.cpp test_sliding_window_log.cpp test_sliding_window_counter.cpp -o example
```

## Running the Tests

```bash
./example
```

This will run all test suites for each rate limiter algorithm.

## Rate Limiter Algorithms

### 1. Token Bucket

**Best for**: Allowing bursts while maintaining average rate

The Token Bucket algorithm maintains a bucket with tokens that are refilled at a constant rate. Requests consume tokens, allowing bursts up to the bucket capacity.

#### Usage

```cpp
#include "token_bucket.h"

// Create a rate limiter: 10 tokens capacity, 2 tokens/second refill rate
TokenBucket limiter(10.0, 2.0);

if (limiter.tryConsume()) {
    // Request allowed
} else {
    // Rate limited
}
```

#### API

- `TokenBucket(double capacity, double refillRate)`
- `bool tryConsume()` / `bool tryConsume(int tokens)`
- `double getAvailableTokens()`
- `double getCapacity()` / `double getRefillRate()`
- `void reset()`

#### Characteristics

- ✅ Allows bursts up to capacity
- ✅ Smooth token refill
- ✅ O(1) time complexity
- ⚠️ Can allow bursts at boundaries

---

### 2. Leaking Bucket

**Best for**: Smooth, constant output rate

The Leaking Bucket algorithm maintains a queue of requests that are processed (leaked) at a fixed rate. Provides smoother output than token bucket.

#### Usage

```cpp
#include "leaking_bucket.h"

// Create a rate limiter: 10 requests capacity, 2 requests/second leak rate
LeakingBucket limiter(10, 2.0);

if (limiter.tryAdd()) {
    // Request allowed
} else {
    // Rate limited (queue full)
}
```

#### API

- `LeakingBucket(int capacity, double leakRate)`
- `bool tryAdd()` / `bool tryAdd(int count)`
- `int getQueueSize()`
- `int getCapacity()` / `double getLeakRate()`
- `void reset()`

#### Characteristics

- ✅ Smooth, constant output rate
- ✅ No bursts at boundaries
- ✅ Queue-based processing
- ⚠️ Rejects when queue is full

---

### 3. Fixed Window

**Best for**: Simple use cases with acceptable boundary bursts

The Fixed Window algorithm divides time into fixed intervals and counts requests within each window. Simple but can allow bursts at window boundaries.

#### Usage

```cpp
#include "fixed_window.h"

// Create a rate limiter: 5 requests per 1 second window
FixedWindow limiter(5, 1);

if (limiter.tryAllow()) {
    // Request allowed
} else {
    // Rate limited
}
```

#### API

- `FixedWindow(int maxRequests, int windowSizeSeconds)`
- `bool tryAllow()` / `bool tryAllow(int count)`
- `int getCurrentCount()`
- `int getMaxRequests()` / `int getWindowSizeSeconds()`
- `double getTimeRemainingInWindow()`
- `void reset()`

#### Characteristics

- ✅ Simple implementation
- ✅ Low memory usage
- ✅ O(1) time complexity
- ⚠️ Can allow bursts at window boundaries
- ⚠️ Less accurate than sliding windows

---

### 4. Sliding Window Log

**Best for**: Maximum accuracy, when memory usage is not a concern

The Sliding Window Log algorithm maintains a log of all request timestamps and removes expired ones. Provides the most accurate rate limiting.

#### Usage

```cpp
#include "sliding_window_log.h"

// Create a rate limiter: 5 requests per 1 second window
SlidingWindowLog limiter(5, 1);

if (limiter.tryAllow()) {
    // Request allowed
} else {
    // Rate limited
}
```

#### API

- `SlidingWindowLog(int maxRequests, int windowSizeSeconds)`
- `bool tryAllow()` / `bool tryAllow(int count)`
- `int getCurrentCount()`
- `int getMaxRequests()` / `int getWindowSizeSeconds()`
- `double getTimeUntilOldestExpires()`
- `void reset()`

#### Characteristics

- ✅ Most accurate rate limiting
- ✅ True sliding window behavior
- ✅ No boundary bursts
- ⚠️ Higher memory usage (stores all timestamps)
- ⚠️ O(n) cleanup where n is requests in window

---

### 5. Sliding Window Counter

**Best for**: Balance between accuracy and memory efficiency

The Sliding Window Counter algorithm divides the window into sub-windows and uses weighted counting. More memory efficient than sliding window log with good accuracy.

#### Usage

```cpp
#include "sliding_window_counter.h"

// Create a rate limiter: 5 requests per 1 second window, 10 sub-windows
SlidingWindowCounter limiter(5, 1, 10);

if (limiter.tryAllow()) {
    // Request allowed
} else {
    // Rate limited
}
```

#### API

- `SlidingWindowCounter(int maxRequests, int windowSizeSeconds, int numSubWindows = 10)`
- `bool tryAllow()` / `bool tryAllow(int count)`
- `double getCurrentCount()` (returns weighted count)
- `int getMaxRequests()` / `int getWindowSizeSeconds()` / `int getNumSubWindows()`
- `void reset()`

#### Characteristics

- ✅ Memory efficient (fixed number of counters)
- ✅ True sliding window behavior
- ✅ Configurable precision (via sub-window count)
- ✅ Good balance of accuracy and memory
- ⚠️ Slight approximation (weighted counting)

---

## Algorithm Comparison

| Algorithm | Accuracy | Memory | Burst Handling | Complexity | Best Use Case |
|-----------|----------|--------|----------------|------------|---------------|
| **Token Bucket** | Medium | Low | ✅ Allows bursts | O(1) | General purpose, burst tolerance |
| **Leaking Bucket** | High | Medium | ❌ No bursts | O(1) | Smooth output rate needed |
| **Fixed Window** | Low | Low | ⚠️ Boundary bursts | O(1) | Simple, low overhead |
| **Sliding Window Log** | Very High | High | ❌ No bursts | O(n) | Maximum accuracy required |
| **Sliding Window Counter** | High | Low | ❌ No bursts | O(k)* | Balance of accuracy/memory |

*Where k is the number of sub-windows (typically 10-20)

## When to Use Which Algorithm?

### Token Bucket
- You need to allow bursts of traffic
- Average rate limiting is acceptable
- Low memory overhead is important

### Leaking Bucket
- You need smooth, constant output rate
- Bursts should be smoothed out
- Queue-based processing is acceptable

### Fixed Window
- Simplicity is more important than accuracy
- Boundary bursts are acceptable
- Very low overhead is required

### Sliding Window Log
- Maximum accuracy is required
- Memory usage is not a concern
- You need true sliding window behavior

### Sliding Window Counter
- You need good accuracy with low memory
- True sliding window behavior is required
- Configurable precision is useful

## Thread Safety

All rate limiters are thread-safe and can be used concurrently from multiple threads. They use `std::mutex` internally to protect shared state.

## Performance Considerations

- **Token Bucket**: O(1) operations, very fast
- **Leaking Bucket**: O(1) operations, very fast
- **Fixed Window**: O(1) operations, very fast
- **Sliding Window Log**: O(n) cleanup where n is requests in window
- **Sliding Window Counter**: O(k) where k is number of sub-windows (typically 10-20)

## Example Output

When running the test suite, you'll see output for each algorithm demonstrating:
- Basic usage and request handling
- Rate limiting behavior
- Time-based expiration
- Burst capacity handling
- Thread-safe concurrent access
- Algorithm-specific features

## License

This is a reference implementation for educational purposes.

## Contributing

Feel free to extend this library with additional rate limiting algorithms or improvements to existing implementations.
