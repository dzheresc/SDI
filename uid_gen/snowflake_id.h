#ifndef SNOWFLAKE_ID_H
#define SNOWFLAKE_ID_H

#include <cstdint>
#include <chrono>
#include <mutex>
#include <atomic>

/**
 * Snowflake ID Generator
 * 
 * Generates unique 64-bit IDs for distributed systems.
 * 
 * ID Structure (64 bits):
 * - 41 bits: Timestamp (milliseconds since custom epoch)
 * - 10 bits: Machine/Node ID
 * - 12 bits: Sequence number
 * 
 * This allows for:
 * - ~69 years of unique IDs (from custom epoch)
 * - 1024 unique machines/nodes
 * - 4096 IDs per millisecond per machine
 */
class SnowflakeIdGenerator {
public:
    /**
     * Constructor
     * @param machineId Machine/Node ID (0-1023)
     * @param epoch Custom epoch in milliseconds (default: 2020-01-01 00:00:00 UTC)
     */
    explicit SnowflakeIdGenerator(
        uint16_t machineId,
        int64_t epoch = 1577836800000LL  // 2020-01-01 00:00:00 UTC in milliseconds
    );
    
    /**
     * Generate a new unique ID
     * @return 64-bit unique ID
     */
    int64_t nextId();
    
    /**
     * Get the machine ID
     * @return Machine ID
     */
    uint16_t getMachineId() const;
    
    /**
     * Parse an ID to extract its components
     * @param id The ID to parse
     * @param timestamp Output: timestamp in milliseconds
     * @param machineId Output: machine ID
     * @param sequence Output: sequence number
     */
    static void parseId(
        int64_t id,
        int64_t& timestamp,
        uint16_t& machineId,
        uint16_t& sequence
    );
    
    /**
     * Get timestamp from an ID
     * @param id The ID
     * @return Timestamp in milliseconds since epoch
     */
    static int64_t getTimestamp(int64_t id);
    
    /**
     * Get machine ID from an ID
     * @param id The ID
     * @return Machine ID
     */
    static uint16_t getMachineIdFromId(int64_t id);
    
    /**
     * Get sequence number from an ID
     * @param id The ID
     * @return Sequence number
     */
    static uint16_t getSequenceFromId(int64_t id);

private:
    static constexpr uint16_t MACHINE_ID_BITS = 10;
    static constexpr uint16_t SEQUENCE_BITS = 12;
    static constexpr uint16_t MAX_MACHINE_ID = (1 << MACHINE_ID_BITS) - 1;  // 1023
    static constexpr uint16_t MAX_SEQUENCE = (1 << SEQUENCE_BITS) - 1;      // 4095
    
    static constexpr uint16_t MACHINE_ID_SHIFT = SEQUENCE_BITS;
    static constexpr uint16_t TIMESTAMP_SHIFT = SEQUENCE_BITS + MACHINE_ID_BITS;
    
    static constexpr int64_t TIMESTAMP_MASK = ((1LL << 41) - 1) << TIMESTAMP_SHIFT;
    static constexpr int64_t MACHINE_ID_MASK = ((1LL << MACHINE_ID_BITS) - 1) << MACHINE_ID_SHIFT;
    static constexpr int64_t SEQUENCE_MASK = (1LL << SEQUENCE_BITS) - 1;
    
    uint16_t machineId_;
    int64_t epoch_;
    
    std::mutex mutex_;
    int64_t lastTimestamp_;
    uint16_t sequence_;
    
    /**
     * Get current timestamp in milliseconds
     */
    int64_t currentTimestamp() const;
    
    /**
     * Wait until next millisecond
     */
    int64_t waitNextMillis(int64_t lastTimestamp);
};

#endif // SNOWFLAKE_ID_H

