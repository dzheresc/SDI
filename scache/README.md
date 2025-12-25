# StringsCache - String Interning Cache

A C++ implementation of a string interning cache that stores each unique string only once in memory blocks and returns lightweight `CachedString` indices for efficient access.

## Overview

String interning is a technique where identical strings are stored only once in memory, and all references point to the same memory location. This provides:

- **Memory Efficiency**: Reduces memory usage when many duplicate strings exist
- **Fast Lookup**: Uses hash map for O(1) average case lookup
- **Block-Based Storage**: Efficient memory management using 64KB blocks
- **Lightweight References**: Returns `CachedString` (just an index) instead of full string copies

## Features

- **String Interning**: Each unique string is stored only once
- **Block-Based Memory**: Uses 64KB memory blocks for efficient allocation
- **CachedString Interface**: Returns lightweight index-based references
- **Fast Hash Function**: Uses xxh64 for fast hashing
- **Memory Alignment**: Automatically aligns strings for optimal performance
- **Simple API**: Minimal, focused interface

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
g++ -std=c++17 example_comprehensive.cpp test_strings_cache_comprehensive.cpp -o example_comprehensive
```

Note: Requires `xxh64.hpp` header file for the hash function.

## Usage

### Basic Example

```cpp
#include "scache.h"

StringsCache cache;

// Intern strings - returns CachedString (index)
CachedString cached1 = cache.intern("hello");
CachedString cached2 = cache.intern("world");
CachedString cached3 = cache.intern("hello");  // Duplicate

// Resolve CachedString to string_view when needed
std::string_view view1 = cache.resolve(cached1);
std::string_view view2 = cache.resolve(cached2);
std::string_view view3 = cache.resolve(cached3);

// view1 and view3 point to the same memory
if (view1.data() == view3.data()) {
    std::cout << "Same memory location!" << std::endl;
}

// Use the views
std::cout << view1 << " " << view2 << std::endl;
```

### Duplicate Detection

```cpp
StringsCache cache;

CachedString cached1 = cache.intern("test");
CachedString cached2 = cache.intern("test");  // Duplicate

// Both return the same index
if (cached1.index == cached2.index) {
    std::cout << "Duplicate strings share same index" << std::endl;
}

// Resolve to verify they're the same
std::string_view v1 = cache.resolve(cached1);
std::string_view v2 = cache.resolve(cached2);
assert(v1.data() == v2.data());  // Same memory
```

### Empty String Handling

```cpp
StringsCache cache;

// Empty string is automatically interned at construction (index 0)
CachedString empty(0);
std::string_view emptyView = cache.resolve(empty);
assert(emptyView.empty());

// Interning empty string again returns index 0
CachedString empty2 = cache.intern("");
assert(empty2.index == 0);
```

### Working with string_view

```cpp
StringsCache cache;

std::string str = "test_string";
std::string_view sv(str);

// Intern from string_view
CachedString cached = cache.intern(sv);

// Original string can be modified/deleted, resolved view remains valid
str.clear();
str = "different";

std::string_view view = cache.resolve(cached);
assert(view == "test_string");  // Still valid!
```

## API Reference

### Constructor

```cpp
StringsCache()
```

- Creates a new cache with one 64KB memory block
- Automatically interns the empty string at index 0

### Methods

#### String Interning

```cpp
CachedString intern(std::string_view sv)
```

- Interns a string and returns a `CachedString` containing its index
- If the string already exists, returns the existing index
- If the string doesn't fit in current block, allocates a new 64KB block
- Automatically aligns memory for optimal performance

#### Resolving

```cpp
std::string_view resolve(CachedString id) const
```

- Resolves a `CachedString` to a `string_view` pointing to the interned string
- Throws `std::runtime_error` if index is out of range
- Returns a non-owning view that remains valid as long as the cache exists

#### Query Operations

```cpp
size_t size() const
```

- Returns the number of unique strings stored in the cache
- Includes the empty string (index 0) that's interned at construction

```cpp
bool empty() const
```

- Returns `true` if `size() == 0`
- Note: Since empty string is interned at construction, this will always return `false` for a valid cache

### CachedString Struct

```cpp
struct CachedString {
    size_t index;
    CachedString(size_t i);
};
```

- Lightweight wrapper around a `size_t` index
- Only 8 bytes (on 64-bit systems) vs 16 bytes for `string_view`
- Can be stored efficiently in containers
- Index 0 always refers to the empty string

## Internal Structure

The cache maintains:

1. **`vector<unique_ptr<char[]>> blocks`**: Memory blocks (64KB each)
   - Stores actual string data
   - New blocks allocated when current block is full
   - Ensures memory stability for string_views

2. **`vector<string_view> index`**: Index of all interned strings
   - Each `string_view` points into one of the memory blocks
   - Allows O(1) access by index
   - Index 0 is always the empty string

3. **`unordered_map<string_view, size_t> map`**: Hash map for lookup
   - Maps string_view to index
   - Uses xxh64 hash function for fast hashing
   - Transparent hash/equality for efficient lookups

4. **`size_t used`**: Tracks bytes used in current block
   - Used to determine when to allocate new block
   - Automatically aligned after each string

## How It Works

1. **Construction**: Allocates first 64KB block and interns empty string
2. **Intern Request**: 
   - Checks if string fits in current block (if not, allocates new block)
   - Looks up string in hash map
   - If found, returns existing index
   - If not found, copies string to current block, creates string_view, adds to index and map
   - Aligns memory pointer
   - Returns new index
3. **Resolve**: Looks up index in `index` vector and returns the `string_view`

## Memory Efficiency

### Block-Based Allocation

- Strings are stored in 64KB blocks
- When a block is full, a new one is allocated
- Memory is aligned for optimal performance
- No fragmentation from individual allocations

### Example

Without interning:
```cpp
std::vector<std::string> strings;
for (int i = 0; i < 10000; ++i) {
    strings.push_back("duplicate_string");  // 10000 copies
}
// Memory: 10000 * string_size + overhead
```

With interning:
```cpp
StringsCache cache;
std::vector<CachedString> cached;
for (int i = 0; i < 10000; ++i) {
    cached.push_back(cache.intern("duplicate_string"));  // Only 1 copy stored
}
// Memory: 1 * string_size + block overhead
// CachedString storage: 10000 * 8 bytes (vs 10000 * 16 bytes for string_view)
```

## Performance Characteristics

- **Intern Operation**: O(1) average case (hash map lookup + block allocation if needed)
- **Resolve Operation**: O(1) (vector index access)
- **Memory**: O(n) where n is number of unique strings
- **Block Allocation**: O(1) amortized (new 64KB block when needed)

## Hash Function

The implementation uses **xxh64** (xxHash 64-bit) for hashing:
- Extremely fast (faster than std::hash)
- Good distribution properties
- Low collision rate
- Suitable for high-performance applications

## Use Cases

- **Configuration Parsing**: Many duplicate configuration keys/values
- **Logging**: Repeated log message formats
- **Database Queries**: Repeated query strings
- **Web Servers**: Repeated URL paths, headers
- **Compilers**: Symbol table, string literals
- **Game Engines**: Resource names, asset paths
- **Serialization**: Repeated field names

## Advantages

✅ **Memory Savings**: Significant reduction when many duplicate strings exist  
✅ **Block-Based**: Efficient memory allocation without fragmentation  
✅ **Lightweight References**: `CachedString` is only 8 bytes  
✅ **Fast Hashing**: xxh64 provides excellent performance  
✅ **Simple API**: Minimal, focused interface  
✅ **Memory Stability**: string_views remain valid as long as cache exists  

## Limitations

⚠️ **Memory Growth**: Cache grows with number of unique strings (never shrinks)  
⚠️ **No Individual Removal**: Cannot remove individual strings (cache grows monotonically)  
⚠️ **Block Overhead**: Each 64KB block may have unused space  
⚠️ **string_view Lifetime**: `string_view` references are valid as long as cache exists  
⚠️ **Index Validation**: `resolve()` throws if index is invalid (no bounds checking in CachedString)  

## Best Practices

1. **Lifetime Management**: Ensure cache outlives all `string_view` references from `resolve()`
2. **Index Validation**: Always validate `CachedString` indices before calling `resolve()` in production code
3. **Memory Monitoring**: Monitor `size()` to track number of unique strings
4. **Reuse Cache**: Reuse the same cache instance for related operations to maximize interning benefits

## Example: Configuration Parser

```cpp
class ConfigParser {
    StringsCache cache_;
    
public:
    CachedString parseKey(const std::string& line) {
        size_t pos = line.find('=');
        std::string key = line.substr(0, pos);
        return cache_.intern(key);  // Intern the key
    }
    
    CachedString parseValue(const std::string& line) {
        size_t pos = line.find('=');
        std::string value = line.substr(pos + 1);
        return cache_.intern(value);  // Intern the value
    }
    
    void printKey(CachedString key) {
        std::string_view view = cache_.resolve(key);
        std::cout << "Key: " << view << std::endl;
    }
};
```

## Example: Symbol Table

```cpp
class SymbolTable {
    StringsCache cache_;
    std::unordered_map<CachedString, SymbolInfo> symbols_;
    
public:
    CachedString addSymbol(const std::string& name) {
        CachedString cached = cache_.intern(name);
        symbols_[cached] = SymbolInfo{/* ... */};
        return cached;
    }
    
    bool hasSymbol(CachedString name) const {
        return symbols_.find(name) != symbols_.end();
    }
    
    std::string_view getName(CachedString name) const {
        return cache_.resolve(name);
    }
};
```

## Running Tests

```bash
./example_comprehensive
```

The comprehensive test suite includes:
- Constructor and initialization
- Basic interning operations
- Duplicate detection
- Resolve functionality
- Error handling
- Memory block management
- Concurrent access
- Large scale operations
- Special characters
- Very long strings

## License

This is a reference implementation for educational purposes.
