# Snowflake ID Generator

A C++ implementation of a distributed unique ID generator that produces 64-bit IDs suitable for distributed systems.

## Overview

This implementation is based on Twitter's Snowflake algorithm, which generates unique 64-bit IDs that are:
- **Unique**: Guaranteed to be unique across distributed systems
- **Time-ordered**: IDs are roughly time-ordered (monotonically increasing)
- **Distributed**: Each machine/node can generate IDs independently
- **Efficient**: High throughput (millions of IDs per second)

## ID Structure

The 64-bit ID is composed of:

```
+--------------------------------------------------------------------------+
| 1 bit (unused) | 41 bits (timestamp) | 10 bits (machine) | 12 bits (seq)|
+--------------------------------------------------------------------------+
```

- **41 bits - Timestamp**: Milliseconds since custom epoch (~69 years)
- **10 bits - Machine ID**: Unique identifier for each machine/node (0-1023)
- **12 bits - Sequence**: Sequence number within the same millisecond (0-4095)

### Capacity

- **Time Range**: ~69 years from custom epoch
- **Machines**: Up to 1,024 unique machines/nodes
- **Throughput**: Up to 4,096 IDs per millisecond per machine
- **Total Capacity**: ~4.2 million IDs per second per machine

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
g++ -std=c++17 -pthread example.cpp snowflake_id.cpp test_snowflake.cpp -o example
```

## Usage

### Basic Example

```cpp
#include "snowflake_id.h"

// Create a generator with machine ID 1
SnowflakeIdGenerator generator(1);

// Generate unique IDs
int64_t id1 = generator.nextId();
int64_t id2 = generator.nextId();
int64_t id3 = generator.nextId();

std::cout << "ID 1: " << id1 << std::endl;
std::cout << "ID 2: " << id2 << std::endl;
std::cout << "ID 3: " << id3 << std::endl;
```

### Multiple Machines

```cpp
// Machine 1
SnowflakeIdGenerator generator1(1);
int64_t id1 = generator1.nextId();

// Machine 2
SnowflakeIdGenerator generator2(2);
int64_t id2 = generator2.nextId();

// Machine 3
SnowflakeIdGenerator generator3(3);
int64_t id3 = generator3.nextId();
```

### Custom Epoch

```cpp
// Custom epoch: 2024-01-01 00:00:00 UTC (in milliseconds)
int64_t customEpoch = 1704067200000LL;
SnowflakeIdGenerator generator(1, customEpoch);

int64_t id = generator.nextId();
```

### Parsing IDs

```cpp
int64_t id = generator.nextId();

// Parse the ID
int64_t timestamp;
uint16_t machineId;
uint16_t sequence;

SnowflakeIdGenerator::parseId(id, timestamp, machineId, sequence);

std::cout << "Timestamp: " << timestamp << " ms" << std::endl;
std::cout << "Machine ID: " << machineId << std::endl;
std::cout << "Sequence: " << sequence << std::endl;

// Or use static methods
int64_t ts = SnowflakeIdGenerator::getTimestamp(id);
uint16_t mid = SnowflakeIdGenerator::getMachineIdFromId(id);
uint16_t seq = SnowflakeIdGenerator::getSequenceFromId(id);
```

## API Reference

### Constructor

```cpp
SnowflakeIdGenerator(uint16_t machineId, int64_t epoch = 1577836800000LL)
```

- `machineId`: Unique machine/node identifier (0-1023)
- `epoch`: Custom epoch in milliseconds (default: 2020-01-01 00:00:00 UTC)

### Methods

#### ID Generation

- `int64_t nextId()`: Generate a new unique ID
- `uint16_t getMachineId()`: Get the machine ID

#### ID Parsing

- `static void parseId(int64_t id, int64_t& timestamp, uint16_t& machineId, uint16_t& sequence)`: Parse an ID into components
- `static int64_t getTimestamp(int64_t id)`: Extract timestamp from ID
- `static uint16_t getMachineIdFromId(int64_t id)`: Extract machine ID from ID
- `static uint16_t getSequenceFromId(int64_t id)`: Extract sequence from ID

## Running the Tests

```bash
./example
```

The test suite includes:
- Basic ID generation
- Uniqueness verification
- ID parsing
- Multiple machines
- High throughput
- Concurrent generation
- ID structure verification
- Sequence rollover
- Custom epoch
- Monotonicity

## Characteristics

### Uniqueness

- IDs are guaranteed to be unique within a single machine
- IDs from different machines are also unique (different machine IDs)
- Even if two machines generate IDs at the exact same time, they have different machine IDs

### Time Ordering

- IDs are roughly time-ordered (monotonically increasing)
- Within the same millisecond, sequence numbers ensure ordering
- IDs from different machines may not be perfectly ordered, but they're close

### Performance

- **Single-threaded**: ~1-2 million IDs/second
- **Multi-threaded**: Scales well with thread count
- **Low latency**: Sub-microsecond per ID generation

## Thread Safety

The generator is thread-safe and can be used concurrently from multiple threads. All operations are protected with mutexes.

## Use Cases

- **Database Primary Keys**: Unique identifiers for database records
- **Distributed Systems**: Unique IDs across multiple servers
- **Message Queues**: Unique message IDs
- **Event Sourcing**: Unique event IDs
- **Microservices**: Request/transaction IDs
- **Caching**: Cache key generation

## Limitations

⚠️ **Machine ID Management**: You must ensure each machine has a unique ID (0-1023). In production, this is typically managed by a configuration service or discovery mechanism.

⚠️ **Clock Synchronization**: Machines should have synchronized clocks (NTP). If clocks drift significantly, ID uniqueness may be affected.

⚠️ **Time Range**: Limited to ~69 years from the epoch. After that, you'll need to adjust the epoch or use a different algorithm.

⚠️ **Sequence Overflow**: If more than 4,096 IDs are needed in a single millisecond, the generator will wait for the next millisecond.

## Comparison with Other ID Generators

| Feature | Snowflake | UUID v4 | Auto-increment |
|---------|-----------|---------|----------------|
| Size | 64 bits | 128 bits | 32/64 bits |
| Uniqueness | Distributed | Global | Single DB |
| Time-ordered | Yes | No | Yes |
| Distributed | Yes | Yes | No |
| Performance | Very High | High | Very High |

## Best Practices

1. **Machine ID Assignment**: Use a centralized service or configuration to assign unique machine IDs
2. **Clock Synchronization**: Ensure all machines use NTP for clock synchronization
3. **Epoch Selection**: Choose an epoch that gives you enough time range for your use case
4. **Monitoring**: Monitor sequence rollover to detect high-throughput scenarios
5. **Error Handling**: Handle clock backward movement exceptions appropriately

## Example: Distributed System Setup

```cpp
// Configuration service provides machine ID
uint16_t machineId = getMachineIdFromConfigService();

// Create generator
SnowflakeIdGenerator generator(machineId);

// Use in your application
class UserService {
    SnowflakeIdGenerator& idGen_;
public:
    UserService(SnowflakeIdGenerator& gen) : idGen_(gen) {}
    
    User createUser(const std::string& name) {
        int64_t userId = idGen_.nextId();
        // Create user with unique ID
        return User(userId, name);
    }
};
```

## License

This is a reference implementation for educational purposes.

