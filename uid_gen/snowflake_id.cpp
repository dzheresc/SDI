#include "snowflake_id.h"
#include <stdexcept>
#include <thread>
#include <chrono>

SnowflakeIdGenerator::SnowflakeIdGenerator(uint16_t machineId, int64_t epoch)
    : machineId_(machineId)
    , epoch_(epoch)
    , lastTimestamp_(-1)
    , sequence_(0)
{
    if (machineId > MAX_MACHINE_ID) {
        throw std::invalid_argument("Machine ID must be between 0 and " + 
                                   std::to_string(MAX_MACHINE_ID));
    }
}

int64_t SnowflakeIdGenerator::nextId() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    int64_t timestamp = currentTimestamp();
    
    // Handle clock going backwards (shouldn't happen, but safety check)
    if (timestamp < lastTimestamp_) {
        throw std::runtime_error("Clock moved backwards. Refusing to generate ID.");
    }
    
    // If same millisecond, increment sequence
    if (timestamp == lastTimestamp_) {
        sequence_ = (sequence_ + 1) & MAX_SEQUENCE;
        
        // Sequence overflow - wait for next millisecond
        if (sequence_ == 0) {
            timestamp = waitNextMillis(lastTimestamp_);
        }
    } else {
        // New millisecond - reset sequence
        sequence_ = 0;
    }
    
    lastTimestamp_ = timestamp;
    
    // Build the ID
    int64_t id = ((timestamp - epoch_) << TIMESTAMP_SHIFT) |
                 (static_cast<int64_t>(machineId_) << MACHINE_ID_SHIFT) |
                 static_cast<int64_t>(sequence_);
    
    return id;
}

uint16_t SnowflakeIdGenerator::getMachineId() const {
    return machineId_;
}

void SnowflakeIdGenerator::parseId(int64_t id, int64_t& timestamp, 
                                    uint16_t& machineId, uint16_t& sequence) {
    timestamp = ((id & TIMESTAMP_MASK) >> TIMESTAMP_SHIFT);
    machineId = static_cast<uint16_t>((id & MACHINE_ID_MASK) >> MACHINE_ID_SHIFT);
    sequence = static_cast<uint16_t>(id & SEQUENCE_MASK);
}

int64_t SnowflakeIdGenerator::getTimestamp(int64_t id) {
    return ((id & TIMESTAMP_MASK) >> TIMESTAMP_SHIFT);
}

uint16_t SnowflakeIdGenerator::getMachineIdFromId(int64_t id) {
    return static_cast<uint16_t>((id & MACHINE_ID_MASK) >> MACHINE_ID_SHIFT);
}

uint16_t SnowflakeIdGenerator::getSequenceFromId(int64_t id) {
    return static_cast<uint16_t>(id & SEQUENCE_MASK);
}

int64_t SnowflakeIdGenerator::currentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return milliseconds;
}

int64_t SnowflakeIdGenerator::waitNextMillis(int64_t lastTimestamp) {
    int64_t timestamp = currentTimestamp();
    while (timestamp <= lastTimestamp) {
        timestamp = currentTimestamp();
        // Small sleep to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    return timestamp;
}

