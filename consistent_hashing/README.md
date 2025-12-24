# Consistent Hashing Implementation

A C++ implementation of the consistent hashing algorithm for distributed systems.

## Overview

Consistent hashing is a distributed hashing scheme that provides:
- **Minimal remapping**: When nodes are added or removed, only a small portion of keys need to be remapped
- **Load balancing**: Keys are distributed evenly across nodes
- **Scalability**: Easy to add or remove nodes without major disruption
- **Virtual nodes**: Support for virtual nodes to improve distribution

## Features

- **Thread-safe**: All operations are protected with mutexes
- **Virtual nodes**: Configurable number of virtual nodes per physical node
- **Replication support**: Get multiple nodes for a key (useful for replication)
- **Distribution statistics**: Analyze key distribution across nodes
- **Efficient**: O(log n) lookup time where n is the number of virtual nodes

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
g++ -std=c++17 -pthread example.cpp consistent_hash.cpp test_consistent_hash.cpp -o example
```

## Usage

### Basic Example

```cpp
#include "consistent_hash.h"

// Create a hash ring with 150 virtual nodes per physical node
ConsistentHash hashRing(150);

// Add nodes
hashRing.addNode("server1");
hashRing.addNode("server2");
hashRing.addNode("server3");

// Get the node responsible for a key
std::string key = "user_12345";
std::string node = hashRing.getNode(key);
std::cout << "Key '" << key << "' is assigned to: " << node << std::endl;
```

### Adding and Removing Nodes

```cpp
// Add a new node
hashRing.addNode("server4");

// Remove a node
hashRing.removeNode("server2");

// Check if a node exists
if (hashRing.hasNode("server1")) {
    std::cout << "server1 is in the ring" << std::endl;
}
```

### Replication

```cpp
// Get multiple nodes for replication (e.g., 3 replicas)
std::vector<std::string> replicas = hashRing.getNodes("key123", 3);
for (const auto& replica : replicas) {
    std::cout << "Replica: " << replica << std::endl;
}
```

### Distribution Analysis

```cpp
// Get distribution statistics
auto stats = hashRing.getDistributionStats(10000);
for (const auto& pair : stats) {
    std::cout << pair.first << ": " << pair.second << " keys" << std::endl;
}
```

## API Reference

### Constructor

```cpp
ConsistentHash(int virtualNodesPerNode = 150)
```

- `virtualNodesPerNode`: Number of virtual nodes per physical node (default: 150)
  - More virtual nodes = better distribution but more memory
  - Recommended: 100-200 for most use cases

### Methods

#### Node Management

- `void addNode(const std::string& nodeName)`: Add a node to the hash ring
- `bool removeNode(const std::string& nodeName)`: Remove a node from the ring
- `bool hasNode(const std::string& nodeName)`: Check if a node exists
- `std::vector<std::string> getAllNodes()`: Get all node names
- `void clear()`: Remove all nodes

#### Key Lookup

- `std::string getNode(const std::string& key)`: Get the node responsible for a key
- `std::vector<std::string> getNodes(const std::string& key, int count)`: Get multiple nodes for replication

#### Information

- `size_t getNodeCount()`: Get number of physical nodes
- `size_t getVirtualNodeCount()`: Get number of virtual nodes
- `std::map<std::string, size_t> getDistributionStats(size_t numTestKeys)`: Get key distribution statistics

## How It Works

### Hash Ring

1. **Virtual Nodes**: Each physical node is represented by multiple virtual nodes on the hash ring
2. **Hash Function**: Uses FNV-1a hash function to map keys and nodes to 32-bit hash values
3. **Ring Structure**: Hash values are arranged in a circle (ring)
4. **Key Assignment**: A key is assigned to the first node encountered when moving clockwise from the key's hash position

### Example

```
Hash Ring (simplified):
    0
    |
1000|     node1 (hash: 500)
    |    /
    |   /
2000|  /  node2 (hash: 1500)
    | /
    |/
3000|     node3 (hash: 2500)
    |
    
Key "user123" (hash: 1200) -> assigned to node2
```

### Virtual Nodes

Virtual nodes improve distribution by:
- Reducing the impact of node removal/addition
- Providing better load balancing
- Allowing fine-grained control over distribution

Each physical node appears multiple times on the ring (as virtual nodes), making the distribution more uniform.

## Running the Tests

```bash
./example
```

The test suite includes:
- Basic operations (add, remove, lookup)
- Node addition and removal
- Key distribution analysis
- Replication support
- Consistency verification
- Minimal remapping verification
- Thread safety
- Virtual nodes configuration
- Edge cases

## Performance Characteristics

- **Lookup**: O(log n) where n is the number of virtual nodes
- **Add Node**: O(v) where v is virtual nodes per node
- **Remove Node**: O(v) where v is virtual nodes per node
- **Memory**: O(n * v) where n is physical nodes and v is virtual nodes per node

## Use Cases

- **Distributed Caching**: Map cache keys to cache servers
- **Load Balancing**: Distribute requests across servers
- **Database Sharding**: Assign data to database nodes
- **CDN**: Route content requests to edge servers
- **Microservices**: Route requests to service instances

## Advantages

✅ **Minimal Remapping**: Only ~1/n keys remapped when adding/removing a node (where n is total nodes)
✅ **Load Balancing**: Even distribution with virtual nodes
✅ **Scalability**: Easy to add/remove nodes
✅ **Consistency**: Same key always maps to same node (when ring is stable)

## Limitations

⚠️ **Memory Usage**: Virtual nodes increase memory usage
⚠️ **Hash Collisions**: Very rare but possible (handled with simple collision resolution)
⚠️ **Non-uniform Distribution**: Without virtual nodes, distribution can be uneven

## Thread Safety

All operations are thread-safe and can be used concurrently from multiple threads. The implementation uses `std::mutex` to protect shared state.

## Hash Function

The implementation uses the FNV-1a (Fowler-Noll-Vo) hash function, which is:
- Fast
- Good distribution properties
- Widely used in practice

## License

This is a reference implementation for educational purposes.

