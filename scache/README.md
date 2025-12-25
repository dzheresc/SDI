# StringsCache - String Interning Cache

A C++ implementation of a string interning cache that stores each unique string only once and returns `string_view` references for efficient access.

## Overview

String interning is a technique where identical strings are stored only once in memory, and all references point to the same memory location. This provides:

- **Memory Efficiency**: Reduces memory usage when many duplicate strings exist
- **Fast Comparison**: Can compare strings by pointer comparison instead of character-by-character
- **string_view Usage**: Returns lightweight `string_view` references instead of full `string` copies

## Features

- **String Interning**: Each unique string is stored only once
- **string_view Interface**: Returns `string_view` for efficient, non-owning references
- **Thread-Safe**: All operations are protected with mutexes
- **Memory Efficient**: Significant memory savings when many duplicate strings exist
- **Index Access**: Access interned strings by index
- **Statistics**: Get memory usage statistics

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
g++ -std=c++17 -pthread example.cpp strings_cache.cpp test_strings_cache.cpp -o example
```

## Usage

### Basic Example

```cpp
#include "strings_cache.h"

StringsCache cache;

// Intern strings
std::string_view view1 = cache.intern("hello");
std::string_view view2 = cache.intern("world");
std::string_view view3 = cache.intern("hello");  // Duplicate

// view1 and view3 point to the same memory
if (view1.data() == view3.data()) {
    std::cout << "Same memory location!" << std::endl;
}

// Use the views
std::cout << view1 << " " << view2 << std::endl;
```

### Different Input Types

```cpp
StringsCache cache;

// From std::string
std::string str = "test";
std::string_view v1 = cache.intern(str);

// From const char*
std::string_view v2 = cache.intern("test");

// From string_view
std::string_view v3 = cache.intern(std::string_view("test"));

// All point to the same memory
assert(v1.data() == v2.data() && v2.data() == v3.data());
```

### Check if String Exists

```cpp
StringsCache cache;

cache.intern("apple");
cache.intern("banana");

if (cache.contains("apple")) {
    std::cout << "Apple is interned" << std::endl;
}

if (!cache.contains("grape")) {
    std::cout << "Grape is not interned" << std::endl;
}
```

### Index Access

```cpp
StringsCache cache;

cache.intern("first");
cache.intern("second");
cache.intern("third");

// Access by index
for (size_t i = 0; i < cache.size(); ++i) {
    std::cout << "[" << i << "] = " << cache[i] << std::endl;
}

// Safe access with bounds checking
try {
    std::string_view view = cache.at(0);
} catch (const std::out_of_range& e) {
    // Handle error
}
```

### Memory Statistics

```cpp
StringsCache cache;

// Intern many strings
for (int i = 0; i < 1000; ++i) {
    cache.intern("duplicate_string");
}

size_t totalChars, totalMemory;
cache.getStats(totalChars, totalMemory);

std::cout << "Total characters: " << totalChars << std::endl;
std::cout << "Total memory: " << totalMemory << " bytes" << std::endl;
```

## API Reference

### Constructor

```cpp
StringsCache(size_t initialCapacity = 1024)
```

- `initialCapacity`: Initial capacity for string storage

### Methods

#### String Interning

- `std::string_view intern(const std::string& str)`: Intern a string
- `std::string_view intern(std::string_view str)`: Intern from string_view
- `std::string_view intern(const char* str)`: Intern a C-string

#### Query Operations

- `bool contains(std::string_view str)`: Check if string is interned
- `size_t size()`: Get number of unique strings
- `bool empty()`: Check if cache is empty

#### Index Access

- `std::string_view at(size_t index)`: Get string_view by index (with bounds checking)
- `std::string_view operator[](size_t index)`: Get string_view by index (no bounds checking)

#### Management

- `void clear()`: Clear all interned strings
- `void reserve(size_t capacity)`: Reserve capacity
- `void getStats(size_t& totalChars, size_t& totalMemory)`: Get statistics

## Internal Structure

The cache maintains:

1. **`vector<string> stringStorage_`**: Stores the actual string data
   - Ensures memory stability for string_views
   - Each unique string is stored once

2. **`vector<string_view> stringViews_`**: Stores string_view references
   - Points to strings in `stringStorage_`
   - Allows index-based access

3. **`unordered_map<string_view, size_t> viewToIndex_`**: Maps string_view to index
   - Fast lookup to check if string exists
   - Maps to index in `stringViews_` vector

## How It Works

1. **Intern Request**: When a string is interned, the cache checks if it already exists
2. **Lookup**: Uses the hash map to quickly find if the string is already stored
3. **Storage**: If new, stores the string in `stringStorage_` and creates a `string_view`
4. **Indexing**: Adds the view to `stringViews_` and updates the hash map
5. **Return**: Returns the `string_view` pointing to the stored string

## Memory Efficiency

### Example

Without interning:
```cpp
std::vector<std::string> strings;
for (int i = 0; i < 10000; ++i) {
    strings.push_back("duplicate_string");  // 10000 copies
}
// Memory: 10000 * string_size
```

With interning:
```cpp
StringsCache cache;
for (int i = 0; i < 10000; ++i) {
    cache.intern("duplicate_string");  // Only 1 copy stored
}
// Memory: 1 * string_size + overhead
```

## Thread Safety

All operations are thread-safe and can be used concurrently from multiple threads. The implementation uses `std::mutex` to protect shared state.

## Performance Characteristics

- **Intern Operation**: O(1) average case (hash map lookup)
- **Contains Check**: O(1) average case
- **Index Access**: O(1)
- **Memory**: O(n) where n is number of unique strings

## Use Cases

- **Configuration Parsing**: Many duplicate configuration keys/values
- **Logging**: Repeated log message formats
- **Database Queries**: Repeated query strings
- **Web Servers**: Repeated URL paths, headers
- **Compilers**: Symbol table, string literals
- **Game Engines**: Resource names, asset paths

## Advantages

✅ **Memory Savings**: Significant reduction when many duplicate strings exist
✅ **Fast Comparison**: Can compare by pointer (if same cache instance)
✅ **Lightweight References**: `string_view` is cheap to copy
✅ **Thread-Safe**: Safe for concurrent access
✅ **Simple API**: Easy to use

## Limitations

⚠️ **Memory Growth**: Cache grows with number of unique strings (never shrinks until `clear()`)
⚠️ **string_view Lifetime**: `string_view` references are valid as long as cache exists
⚠️ **No Removal**: Individual strings cannot be removed (only `clear()` all)
⚠️ **Hash Collisions**: Very rare, but possible (handled by hash map)

## Best Practices

1. **Lifetime Management**: Ensure cache outlives all `string_view` references
2. **Capacity Planning**: Use `reserve()` if you know approximate number of unique strings
3. **Memory Monitoring**: Use `getStats()` to monitor memory usage
4. **Clear When Done**: Call `clear()` when cache is no longer needed to free memory

## Example: Configuration Parser

```cpp
class ConfigParser {
    StringsCache cache_;
    
public:
    std::string_view parseKey(const std::string& line) {
        // Extract key from line
        size_t pos = line.find('=');
        std::string key = line.substr(0, pos);
        return cache_.intern(key);  // Intern the key
    }
    
    std::string_view parseValue(const std::string& line) {
        // Extract value from line
        size_t pos = line.find('=');
        std::string value = line.substr(pos + 1);
        return cache_.intern(value);  // Intern the value
    }
};
```

## License

This is a reference implementation for educational purposes.

