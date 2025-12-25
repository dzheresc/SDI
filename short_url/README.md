# URL Shortener Service

A C++ library for URL shortening with base62 encoding, database-like interface, and CSV persistence.

## Overview

This URL shortener service provides two implementations:

1. **UrlShortener** (unordered_map backend): Simple, in-memory storage
2. **UrlShortenerKV** (KeyValue store backend): Distributed storage with horizontal scaling

Both implementations provide:
- **Base62 Encoding**: Short, URL-safe codes using 0-9, a-z, A-Z
- **Database Interface**: Clean API wrapping storage backend
- **CSV Persistence**: Save and load URL mappings to/from CSV files
- **Duplicate Detection**: Same URL always returns same short code
- **Fast Lookup**: O(1) average case for expand operations

## Features

- **Base62 Encoding**: Efficient encoding for short codes
- **Database-like Interface**: Clean API wrapping unordered_map storage
- **CSV File Support**: Save/load functionality for persistence
- **Duplicate Handling**: Automatically handles duplicate URLs
- **Custom Base URL**: Configurable base URL for shortened links
- **Statistics**: Get database statistics

## Building

### Using CMake

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Manual Compilation

**Unordered Map Version:**
```bash
g++ -std=c++17 example.cpp url_shortener.cpp test_url_shortener.cpp -o example
```

**KeyValue Store Version:**
```bash
g++ -std=c++17 -I../key_value_store -I../consistent_hashing \
    example_kv.cpp url_shortener_kv.cpp test_url_shortener_kv.cpp \
    ../key_value_store/kv_store.cpp ../consistent_hashing/consistent_hash.cpp \
    -o example_kv
```

## Usage

### Implementation 1: UrlShortener (unordered_map)

Simple, in-memory storage using unordered_map:

```cpp
#include "url_shortener.h"

UrlShortener shortener;

// Shorten a URL
std::string longUrl = "https://www.example.com/very/long/url/path";
std::string shortUrl = shortener.shorten(longUrl);
std::cout << "Short URL: " << shortUrl << std::endl;
// Output: https://short.ly/1

// Expand a short URL
std::string expanded = shortener.expandUrl(shortUrl);
std::cout << "Original URL: " << expanded << std::endl;
// Output: https://www.example.com/very/long/url/path
```

### Implementation 2: UrlShortenerKV (KeyValue Store)

Distributed storage with horizontal scaling:

```cpp
#include "url_shortener_kv.h"

UrlShortenerKV shortener;

// Add servers for distributed storage
shortener.addServer("server1");
shortener.addServer("server2");
shortener.addServer("server3");

// Shorten a URL (automatically distributed across servers)
std::string longUrl = "https://www.example.com/very/long/url/path";
std::string shortUrl = shortener.shorten(longUrl);

// Check which server stores this URL
std::string shortCode = shortUrl.substr(18);  // Extract code
std::string server = shortener.getServerForKey(shortCode);
std::cout << "URL stored on: " << server << std::endl;

// Expand works the same way
std::string expanded = shortener.expandUrl(shortUrl);
```

### Custom Base URL

```cpp
UrlShortener shortener("https://my.short/");

std::string shortUrl = shortener.shorten("https://www.example.com");
// shortUrl will be: https://my.short/1
```

### Duplicate URLs

```cpp
UrlShortener shortener;

std::string url = "https://www.example.com";
std::string short1 = shortener.shorten(url);
std::string short2 = shortener.shorten(url);  // Same URL

// Both return the same short URL
assert(short1 == short2);
```

### Save and Load

```cpp
UrlShortener shortener;

// Add some URLs
shortener.shorten("https://www.example.com/page1");
shortener.shorten("https://www.example.com/page2");

// Save to CSV file
shortener.saveToFile("urls.csv");

// Later, load from file
UrlShortener shortener2;
shortener2.loadFromFile("urls.csv");

// URLs are restored
std::string expanded = shortener2.expandUrl("https://short.ly/1");
```

### Check if Code Exists

```cpp
UrlShortener shortener;
shortener.shorten("https://www.example.com");

if (shortener.exists("1")) {
    std::cout << "Short code exists" << std::endl;
}
```

## API Reference

### UrlShortener (unordered_map backend)

#### Constructor

```cpp
UrlShortener(const std::string& baseUrl = "https://short.ly/")
```

- `baseUrl`: Base URL for shortened links (default: "https://short.ly/")

### UrlShortenerKV (KeyValue store backend)

#### Constructor

```cpp
UrlShortenerKV(const std::string& baseUrl = "https://short.ly/", int virtualNodesPerNode = 150)
```

- `baseUrl`: Base URL for shortened links (default: "https://short.ly/")
- `virtualNodesPerNode`: Number of virtual nodes per server for consistent hashing (default: 150)

### Methods

#### URL Operations

- `std::string shorten(const std::string& longUrl)`: Shorten a URL
  - Returns: Full shortened URL (baseUrl + short code)
  - Throws: `std::invalid_argument` if URL is empty
  - Duplicate URLs return the same short URL

- `std::string expand(const std::string& shortCode)`: Expand a short code
  - Returns: Original URL, or empty string if not found
  - `shortCode`: Just the code part (e.g., "1", not the full URL)

- `std::string expandUrl(const std::string& shortUrl)`: Expand a full short URL
  - Returns: Original URL, or empty string if not found
  - `shortUrl`: Full shortened URL (e.g., "https://short.ly/1")

#### Query Operations

- `bool exists(const std::string& shortCode)`: Check if short code exists
- `size_t size()`: Get number of shortened URLs
- `bool empty()`: Check if database is empty
- `void clear()`: Clear all URLs

#### Persistence

- `bool saveToFile(const std::string& filename)`: Save to CSV file
  - Returns: `true` if successful, `false` otherwise
  - Format: `short_code,long_url`

- `bool loadFromFile(const std::string& filename)`: Load from CSV file
  - Returns: `true` if successful, `false` otherwise
  - Clears existing data before loading

#### Statistics

- `void getStats(size_t& totalUrls, size_t& totalShortCodes)`: Get statistics
  - `totalUrls`: Number of unique URLs
  - `totalShortCodes`: Number of short codes (same as totalUrls)

#### Server Management (UrlShortenerKV only)

- `bool addServer(const std::string& serverId)`: Add a server to the cluster
- `bool removeServer(const std::string& serverId)`: Remove a server from the cluster
- `std::vector<std::string> getServers()`: Get all server identifiers
- `std::string getServerForKey(const std::string& shortCode)`: Get server for a short code

#### Utility (Static Methods)

- `static std::string encodeBase62(uint64_t num)`: Encode number to base62
- `static uint64_t decodeBase62(const std::string& encoded)`: Decode base62 to number

## Base62 Encoding

Base62 uses 62 characters: `0-9`, `a-z`, `A-Z`

### Examples

- `0` → `"0"`
- `1` → `"1"`
- `10` → `"a"`
- `62` → `"10"` (1*62 + 0)
- `100` → `"1C"`

### Benefits

- **URL-Safe**: No special characters that need encoding
- **Short Codes**: More compact than decimal for large numbers
- **Case-Sensitive**: Distinguishes between uppercase and lowercase

## CSV File Format

The CSV file format is simple:

```csv
short_code,long_url
1,https://www.example.com/page1
2,https://www.example.com/page2
a,https://www.google.com
```

- First line is header: `short_code,long_url`
- Each subsequent line: `code,url`
- No escaping needed for simple URLs (commas in URLs would need special handling)

## Internal Structure

### UrlShortener (unordered_map)

1. **`unordered_map<string, string> urlMap_`**: Maps short code → long URL
   - Fast lookup for expand operations
   - O(1) average case

2. **`unordered_map<string, string> reverseMap_`**: Maps long URL → short code
   - Fast lookup for duplicate detection
   - O(1) average case

3. **`uint64_t nextId_`**: Next ID to encode
   - Sequential IDs ensure uniqueness
   - Encoded to base62 for short codes

### UrlShortenerKV (KeyValue Store)

1. **`KeyValueStore kvStore_`**: Main storage for short code → long URL
   - Uses consistent hashing for distribution
   - Supports multiple servers

2. **`KeyValueStore reverseKvStore_`**: Reverse mapping long URL → short code
   - For duplicate detection
   - Also distributed across servers

3. **`vector<string> shortCodeIndex_`**: Index of all short codes
   - Used for iteration and CSV export
   - Stored in KeyValueStore for persistence

4. **`uint64_t nextId_`**: Next ID to encode
   - Stored in KeyValueStore for persistence
   - Sequential IDs ensure uniqueness

## How It Works

1. **Shorten**:
   - Check if URL already exists (using reverseMap_)
   - If exists, return existing short URL
   - If new, generate next ID, encode to base62
   - Store in both maps
   - Return full short URL

2. **Expand**:
   - Extract short code from URL (if full URL provided)
   - Look up in urlMap_
   - Return original URL

3. **Save**:
   - Write CSV header
   - Write all mappings (short_code,long_url)

4. **Load**:
   - Read CSV file
   - Parse each line
   - Restore mappings
   - Update nextId_ based on highest decoded ID

## Performance Characteristics

- **Shorten**: O(1) average case (hash map lookup)
- **Expand**: O(1) average case (hash map lookup)
- **Save**: O(n) where n is number of URLs
- **Load**: O(n) where n is number of URLs in file
- **Memory**: O(n) where n is number of unique URLs

## Use Cases

- **URL Shortening Services**: Like bit.ly, tinyurl.com
- **Analytics**: Track click-through rates
- **Social Media**: Share shorter links
- **QR Codes**: Shorter URLs for QR code generation
- **Email**: Shorter links in email campaigns
- **API Services**: Provide short URL endpoints

## Advantages

✅ **Fast Lookup**: O(1) average case operations  
✅ **Duplicate Handling**: Same URL always gets same short code  
✅ **Persistence**: Save/load to CSV files  
✅ **Base62 Encoding**: Short, URL-safe codes  
✅ **Simple API**: Easy to use  
✅ **Memory Efficient**: Only stores unique URLs  

## Limitations

⚠️ **Sequential IDs**: IDs are sequential, not random (predictable)  
⚠️ **No Expiration**: URLs never expire (cache grows)  
⚠️ **CSV Limitations**: Commas in URLs may cause issues (not escaped)  
⚠️ **No Thread Safety**: Not thread-safe (add mutex if needed)  
⚠️ **Memory Growth**: Database grows with number of unique URLs  

## Example: Web Service Integration

```cpp
class UrlShortenerService {
    UrlShortener shortener_;
    
public:
    std::string shortenUrl(const std::string& longUrl) {
        return shortener_.shorten(longUrl);
    }
    
    std::string expandUrl(const std::string& shortUrl) {
        return shortener_.expandUrl(shortUrl);
    }
    
    void saveDatabase() {
        shortener_.saveToFile("urls_database.csv");
    }
    
    void loadDatabase() {
        shortener_.loadFromFile("urls_database.csv");
    }
};
```

## Example: Batch Processing

```cpp
UrlShortener shortener;

// Load existing database
shortener.loadFromFile("existing_urls.csv");

// Process new URLs
std::vector<std::string> newUrls = {
    "https://www.example.com/page1",
    "https://www.example.com/page2",
    // ...
};

for (const auto& url : newUrls) {
    std::string shortUrl = shortener.shorten(url);
    std::cout << url << " -> " << shortUrl << std::endl;
}

// Save updated database
shortener.saveToFile("updated_urls.csv");
```

## Running the Tests

**Unordered Map Version:**
```bash
./example
```

**KeyValue Store Version:**
```bash
./example_kv
```

The test suites include:
- Basic shorten/expand operations
- Duplicate URL handling
- Base62 encoding/decoding
- Multiple URLs
- Save/load functionality
- Roundtrip testing
- Invalid input handling
- Large scale operations
- Server management (KeyValue store version)
- Distributed storage (KeyValue store version)

## Choosing an Implementation

### Use UrlShortener (unordered_map) when:
- Single-process application
- Simple, fast in-memory storage needed
- No need for distributed storage
- Lower memory overhead preferred

### Use UrlShortenerKV (KeyValue store) when:
- Need horizontal scaling
- Multiple servers/nodes required
- Distributed storage needed
- Want to leverage consistent hashing
- Planning for future distribution

## Future Enhancements

- Thread safety with mutexes
- URL expiration/TTL
- Click tracking/analytics
- Custom short code support
- URL validation
- CSV escaping for URLs with commas
- Random/non-sequential IDs
- Database backend (SQLite, etc.)

## License

This is a reference implementation for educational purposes.

