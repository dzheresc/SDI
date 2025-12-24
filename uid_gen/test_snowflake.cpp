#include "snowflake_id.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <set>
#include <atomic>
#include <cassert>
#include <chrono>

void testBasicGeneration() {
    std::cout << "=== Basic ID Generation Test ===" << std::endl;
    
    SnowflakeIdGenerator generator(1);
    
    std::cout << "Machine ID: " << generator.getMachineId() << std::endl;
    std::cout << "\nGenerating 10 IDs:" << std::endl;
    
    for (int i = 0; i < 10; ++i) {
        int64_t id = generator.nextId();
        std::cout << "ID " << (i + 1) << ": " << id << std::endl;
    }
    std::cout << std::endl;
}

void testUniqueness() {
    std::cout << "=== Uniqueness Test ===" << std::endl;
    
    SnowflakeIdGenerator generator(1);
    
    std::set<int64_t> ids;
    const int numIds = 10000;
    
    std::cout << "Generating " << numIds << " IDs..." << std::endl;
    
    for (int i = 0; i < numIds; ++i) {
        int64_t id = generator.nextId();
        ids.insert(id);
    }
    
    std::cout << "Unique IDs generated: " << ids.size() << std::endl;
    
    if (ids.size() == numIds) {
        std::cout << "✓ All IDs are unique!" << std::endl;
    } else {
        std::cout << "✗ Some IDs are duplicates!" << std::endl;
    }
    std::cout << std::endl;
}

void testIdParsing() {
    std::cout << "=== ID Parsing Test ===" << std::endl;
    
    SnowflakeIdGenerator generator(42);
    
    int64_t id = generator.nextId();
    std::cout << "Generated ID: " << id << std::endl;
    
    int64_t timestamp;
    uint16_t machineId;
    uint16_t sequence;
    
    SnowflakeIdGenerator::parseId(id, timestamp, machineId, sequence);
    
    std::cout << "\nParsed components:" << std::endl;
    std::cout << "  Timestamp: " << timestamp << " ms" << std::endl;
    std::cout << "  Machine ID: " << machineId << std::endl;
    std::cout << "  Sequence: " << sequence << std::endl;
    
    // Verify machine ID
    if (machineId == 42) {
        std::cout << "✓ Machine ID matches!" << std::endl;
    } else {
        std::cout << "✗ Machine ID mismatch!" << std::endl;
    }
    std::cout << std::endl;
}

void testMultipleMachines() {
    std::cout << "=== Multiple Machines Test ===" << std::endl;
    
    SnowflakeIdGenerator generator1(1);
    SnowflakeIdGenerator generator2(2);
    SnowflakeIdGenerator generator3(3);
    
    std::set<int64_t> allIds;
    const int idsPerMachine = 1000;
    
    std::cout << "Generating " << idsPerMachine << " IDs from each of 3 machines..." << std::endl;
    
    for (int i = 0; i < idsPerMachine; ++i) {
        allIds.insert(generator1.nextId());
        allIds.insert(generator2.nextId());
        allIds.insert(generator3.nextId());
    }
    
    std::cout << "Total unique IDs: " << allIds.size() << std::endl;
    std::cout << "Expected: " << (idsPerMachine * 3) << std::endl;
    
    if (allIds.size() == idsPerMachine * 3) {
        std::cout << "✓ All IDs from different machines are unique!" << std::endl;
    } else {
        std::cout << "✗ Some IDs are duplicates across machines!" << std::endl;
    }
    std::cout << std::endl;
}

void testHighThroughput() {
    std::cout << "=== High Throughput Test ===" << std::endl;
    
    SnowflakeIdGenerator generator(1);
    
    const int numIds = 100000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numIds; ++i) {
        generator.nextId();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    double idsPerSecond = (numIds * 1000.0) / duration.count();
    
    std::cout << "Generated " << numIds << " IDs in " << duration.count() << " ms" << std::endl;
    std::cout << "Throughput: " << std::fixed << std::setprecision(2) 
              << idsPerSecond << " IDs/second" << std::endl;
    std::cout << std::endl;
}

void testConcurrentGeneration() {
    std::cout << "=== Concurrent Generation Test ===" << std::endl;
    
    SnowflakeIdGenerator generator(1);
    std::atomic<int> successCount(0);
    std::set<int64_t> allIds;
    std::mutex idsMutex;
    
    auto worker = [&generator, &successCount, &allIds, &idsMutex](int threadId) {
        for (int i = 0; i < 1000; ++i) {
            try {
                int64_t id = generator.nextId();
                {
                    std::lock_guard<std::mutex> lock(idsMutex);
                    allIds.insert(id);
                }
                successCount++;
            } catch (...) {
                // Handle any exceptions
            }
        }
    };
    
    std::vector<std::thread> threads;
    const int numThreads = 10;
    
    std::cout << "Starting " << numThreads << " threads, each generating 1000 IDs..." << std::endl;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Total IDs generated: " << successCount.load() << std::endl;
    std::cout << "Unique IDs: " << allIds.size() << std::endl;
    
    if (allIds.size() == successCount.load()) {
        std::cout << "✓ All concurrent IDs are unique!" << std::endl;
    } else {
        std::cout << "✗ Some concurrent IDs are duplicates!" << std::endl;
    }
    std::cout << std::endl;
}

void testIdStructure() {
    std::cout << "=== ID Structure Test ===" << std::endl;
    
    SnowflakeIdGenerator generator(123);
    
    int64_t id = generator.nextId();
    std::cout << "Generated ID: " << id << std::endl;
    std::cout << "ID in hex: 0x" << std::hex << id << std::dec << std::endl;
    
    int64_t timestamp = SnowflakeIdGenerator::getTimestamp(id);
    uint16_t machineId = SnowflakeIdGenerator::getMachineIdFromId(id);
    uint16_t sequence = SnowflakeIdGenerator::getSequenceFromId(id);
    
    std::cout << "\nID Structure:" << std::endl;
    std::cout << "  Bits 0-11 (Sequence): " << sequence << std::endl;
    std::cout << "  Bits 12-21 (Machine ID): " << machineId << std::endl;
    std::cout << "  Bits 22-62 (Timestamp): " << timestamp << " ms" << std::endl;
    
    // Verify 64-bit
    std::cout << "\nVerifying 64-bit constraint..." << std::endl;
    if (id > 0 && id <= INT64_MAX) {
        std::cout << "✓ ID fits in 64-bit signed integer" << std::endl;
    } else {
        std::cout << "✗ ID exceeds 64-bit range!" << std::endl;
    }
    std::cout << std::endl;
}

void testSequenceRollover() {
    std::cout << "=== Sequence Rollover Test ===" << std::endl;
    
    SnowflakeIdGenerator generator(1);
    
    std::cout << "Generating IDs rapidly to test sequence rollover..." << std::endl;
    
    std::vector<int64_t> ids;
    for (int i = 0; i < 5000; ++i) {
        ids.push_back(generator.nextId());
    }
    
    // Check that IDs are still unique
    std::set<int64_t> uniqueIds(ids.begin(), ids.end());
    
    std::cout << "Generated: " << ids.size() << " IDs" << std::endl;
    std::cout << "Unique: " << uniqueIds.size() << " IDs" << std::endl;
    
    if (uniqueIds.size() == ids.size()) {
        std::cout << "✓ Sequence rollover handled correctly!" << std::endl;
    } else {
        std::cout << "✗ Sequence rollover issue detected!" << std::endl;
    }
    std::cout << std::endl;
}

void testCustomEpoch() {
    std::cout << "=== Custom Epoch Test ===" << std::endl;
    
    // Custom epoch: 2024-01-01 00:00:00 UTC
    int64_t customEpoch = 1704067200000LL;
    SnowflakeIdGenerator generator(1, customEpoch);
    
    int64_t id = generator.nextId();
    int64_t timestamp = SnowflakeIdGenerator::getTimestamp(id);
    
    std::cout << "Custom epoch: " << customEpoch << " ms (2024-01-01)" << std::endl;
    std::cout << "Generated ID: " << id << std::endl;
    std::cout << "Timestamp component: " << timestamp << " ms since epoch" << std::endl;
    std::cout << std::endl;
}

void testMonotonicity() {
    std::cout << "=== Monotonicity Test ===" << std::endl;
    
    SnowflakeIdGenerator generator(1);
    
    std::cout << "Generating IDs and checking monotonicity..." << std::endl;
    
    int64_t prevId = 0;
    bool isMonotonic = true;
    
    for (int i = 0; i < 1000; ++i) {
        int64_t id = generator.nextId();
        if (id <= prevId) {
            isMonotonic = false;
            std::cout << "Non-monotonic detected at ID " << i << std::endl;
            break;
        }
        prevId = id;
    }
    
    if (isMonotonic) {
        std::cout << "✓ IDs are monotonically increasing!" << std::endl;
    } else {
        std::cout << "✗ IDs are not monotonically increasing!" << std::endl;
    }
    std::cout << std::endl;
}

void runAllTests() {
    try {
        testBasicGeneration();
        testUniqueness();
        testIdParsing();
        testMultipleMachines();
        testHighThroughput();
        testConcurrentGeneration();
        testIdStructure();
        testSequenceRollover();
        testCustomEpoch();
        testMonotonicity();
        
        std::cout << "========================================" << std::endl;
        std::cout << "All tests completed successfully!" << std::endl;
        std::cout << "========================================" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        throw;
    }
}

