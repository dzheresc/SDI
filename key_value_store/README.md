# Distributed Key-Value Store

A C++ implementation of a distributed key-value store using consistent hashing for horizontal scaling and distribution across multiple servers.

## Overview

This key-value store implementation provides:
- **Horizontal Scaling**: Add or remove servers dynamically
- **Consistent Hashing**: Uses consistent hashing to distribute keys across servers
- **Automatic Distribution**: Keys are automatically assigned to servers based on hash
- **Server Management**: Add, remove, and query servers in the cluster
- **Thread-Safe**: All operations are thread-safe for concurrent access

## Features

- **Distributed Storage**: Keys are distributed across multiple servers using consistent hashing
- **Dynamic Scaling**: Add or remove servers without downtime
- **Key-Value Operations**: Set, get, delete, and check existence of keys
- **Server Tracking**: Track which keys belong to which server
- **Statistics**: Get distribution statistics across servers
- **Thread-Safe**: Concurrent access from multiple threads

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
g++ -std=c++17 -pthread -I../consistent_hashing example.cpp kv_store.cpp test_kv_store.cpp ../consistent_hashing/consistent_hash.cpp -o example
```

## Usage

### Basic Example

```cpp
#include "kv_store.h"

// Create a key-value store
KeyValueStore store;

// Add servers to the cluster
store.addServer("server1");
store.addServer("server2");
store.addServer("server3");

// Store key-value pairs
store.set("user:1001", "John Doe");
store.set("user:1002", "Jane Smith");
store.set("product:2001", "Laptop");

// Retrieve values
std::string user = store.get("user:1001");
std::cout << "User: " << user << std::endl;

// Check if key exists
if (store.exists("user:1001")) {
    std::cout << "Key exists" << std::endl;
}

// Delete a key
store.remove("user:1001");
```

### Server Management

```cpp
// Add a new server
store.addServer("server4");

// Remove a server (keys will be reassigned)
store.removeServer("server2");

// Get all servers
auto servers = store.getServers();
for (const auto& server : servers) {
    std::cout << "Server: " << server << std::endl;
}

// Get server for a specific key
std::string server = store.getServerForKey("user:1001");
std::cout << "Key stored on: " << server << std::endl;
```

### Distribution Statistics

```cpp
// Get distribution statistics
auto stats = store.getStats();
for (const auto& pair : stats) {
    std::cout << pair.first << ": " << pair.second << " keys" << std::endl;
}

// Get keys for a specific server
auto keys = store.getKeysForServer("server1");
std::cout << "Server1 has " << keys.size() << " keys" << std::endl;
```

## API Reference

### Constructor

```cpp
KeyValueStore(int virtualNodesPerNode = 150)
```

- `virtualNodesPerNode`: Number of virtual nodes per server for consistent hashing (default: 150)

### Server Management

- `bool addServer(const std::string& serverId)`: Add a server to the cluster
- `bool removeServer(const std::string& serverId)`: Remove a server from the cluster
- `std::vector<std::string> getServers()`: Get all server identifiers
- `size_t getServerCount()`: Get the number of servers

### Key-Value Operations

- `bool set(const std::string& key, const std::string& value)`: Store a key-value pair
- `std::string get(const std::string& key)`: Retrieve a value by key
- `bool remove(const std::string& key)`: Delete a key-value pair
- `bool exists(const std::string& key)`: Check if a key exists
- `size_t getTotalEntries()`: Get total number of key-value pairs

### Query Operations

- `std::string getServerForKey(const std::string& key)`: Get the server responsible for a key
- `std::vector<std::string> getKeysForServer(const std::string& serverId)`: Get all keys on a server
- `std::map<std::string, size_t> getStats()`: Get distribution statistics
- `void clear()`: Clear all data and servers

## How It Works

### Consistent Hashing

The store uses consistent hashing (from the `ConsistentHash` class) to determine which server should store each key:

1. **Hash Calculation**: Each key is hashed to a 32-bit value
2. **Server Selection**: The hash ring is used to find the server responsible for that hash value
3. **Distribution**: Keys are automatically distributed across available servers
4. **Minimal Remapping**: When servers are added/removed, only a small portion of keys need to be reassigned

### Key Distribution

```
Hash Ring:
    Key "user:1001" (hash: 500)  -> server1
    Key "user:1002" (hash: 1500) -> server2
    Key "user:1003" (hash: 2500) -> server3
```

### Server Addition/Removal

- **Adding a Server**: New keys will be distributed to the new server. Existing keys remain on their current servers (in a real distributed system, rebalancing would occur).
- **Removing a Server**: Keys from the removed server are reassigned to other servers based on the hash ring.

## Running the Tests

```bash
./example
```

The test suite includes:
- Basic operations (set, get, delete)
- Server distribution
- Key-server mapping
- Server addition and removal
- Update operations
- Delete operations
- Thread safety
- Server keys retrieval
- Edge cases

## Architecture

### Components

1. **KeyValueStore**: Main interface for the distributed store
2. **ConsistentHash**: Hash ring implementation for server selection
3. **Data Storage**: In-memory storage (in a real implementation, this would be distributed across servers)

### Thread Safety

All operations are thread-safe and can be used concurrently from multiple threads. The implementation uses `std::mutex` to protect shared state.

## Use Cases

- **Distributed Caching**: Cache data across multiple cache servers
- **Database Sharding**: Distribute database records across nodes
- **Load Balancing**: Route requests to appropriate servers
- **Microservices**: Store configuration or session data
- **CDN**: Distribute content across edge servers

## Limitations

⚠️ **In-Memory Storage**: This implementation stores data in memory. In a real distributed system, each server would have its own storage.

⚠️ **No Replication**: This implementation doesn't include replication. For production use, you'd want to replicate data across multiple servers.

⚠️ **No Persistence**: Data is not persisted to disk. In a real system, you'd want persistent storage.

⚠️ **Simplified Rebalancing**: When servers are removed, keys are reassigned but data migration would need to be handled in a real system.

## Performance Characteristics

- **Set Operation**: O(log n) where n is the number of virtual nodes
- **Get Operation**: O(1) for in-memory lookup
- **Server Addition**: O(v) where v is virtual nodes per server
- **Server Removal**: O(v + k) where v is virtual nodes and k is keys on that server

## Future Enhancements

- Persistent storage (disk-based)
- Replication support
- Data migration on server addition/removal
- Network communication between servers
- Consistency guarantees (strong/eventual)
- Transaction support
- Expiration/TTL for keys

## License

This is a reference implementation for educational purposes.

