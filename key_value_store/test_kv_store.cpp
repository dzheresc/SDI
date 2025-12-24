#include "kv_store.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <atomic>
#include <cassert>

void testBasicOperations() {
    std::cout << "=== Basic Operations Test ===" << std::endl;
    
    KeyValueStore store;
    
    // Add servers
    std::cout << "Adding servers: server1, server2, server3..." << std::endl;
    store.addServer("server1");
    store.addServer("server2");
    store.addServer("server3");
    
    std::cout << "Number of servers: " << store.getServerCount() << std::endl;
    std::cout << std::endl;
    
    // Store some key-value pairs
    std::cout << "Storing key-value pairs..." << std::endl;
    store.set("user:1001", "John Doe");
    store.set("user:1002", "Jane Smith");
    store.set("user:1003", "Bob Johnson");
    store.set("product:2001", "Laptop");
    store.set("product:2002", "Mouse");
    
    std::cout << "Total entries: " << store.getTotalEntries() << std::endl;
    std::cout << std::endl;
    
    // Retrieve values
    std::cout << "Retrieving values:" << std::endl;
    std::cout << "  user:1001 -> " << store.get("user:1001") << std::endl;
    std::cout << "  user:1002 -> " << store.get("user:1002") << std::endl;
    std::cout << "  product:2001 -> " << store.get("product:2001") << std::endl;
    std::cout << std::endl;
    
    // Check existence
    std::cout << "Checking existence:" << std::endl;
    std::cout << "  user:1001 exists: " << (store.exists("user:1001") ? "Yes" : "No") << std::endl;
    std::cout << "  user:9999 exists: " << (store.exists("user:9999") ? "Yes" : "No") << std::endl;
    std::cout << std::endl;
}

void testServerDistribution() {
    std::cout << "=== Server Distribution Test ===" << std::endl;
    
    KeyValueStore store;
    
    store.addServer("server1");
    store.addServer("server2");
    store.addServer("server3");
    store.addServer("server4");
    
    std::cout << "Storing 1000 keys..." << std::endl;
    for (int i = 0; i < 1000; ++i) {
        std::string key = "key_" + std::to_string(i);
        std::string value = "value_" + std::to_string(i);
        store.set(key, value);
    }
    
    std::cout << "\nDistribution across servers:" << std::endl;
    auto stats = store.getStats();
    for (const auto& pair : stats) {
        double percentage = (100.0 * pair.second) / 1000;
        std::cout << "  " << pair.first << ": " << pair.second << " keys ("
                  << std::fixed << std::setprecision(2) << percentage << "%)" << std::endl;
    }
    std::cout << std::endl;
}

void testKeyServerMapping() {
    std::cout << "=== Key-Server Mapping Test ===" << std::endl;
    
    KeyValueStore store;
    
    store.addServer("server1");
    store.addServer("server2");
    store.addServer("server3");
    
    std::vector<std::string> testKeys = {"key1", "key2", "key3", "key4", "key5"};
    
    std::cout << "Key to server mapping:" << std::endl;
    for (const auto& key : testKeys) {
        std::string server = store.getServerForKey(key);
        std::cout << "  " << key << " -> " << server << std::endl;
    }
    std::cout << std::endl;
}

void testServerAddition() {
    std::cout << "=== Server Addition Test ===" << std::endl;
    
    KeyValueStore store;
    
    store.addServer("server1");
    store.addServer("server2");
    store.addServer("server3");
    
    std::cout << "Initial servers: " << store.getServerCount() << std::endl;
    
    // Store some keys
    for (int i = 0; i < 100; ++i) {
        std::string key = "key_" + std::to_string(i);
        store.set(key, "value_" + std::to_string(i));
    }
    
    auto statsBefore = store.getStats();
    std::cout << "\nKeys per server before adding server4:" << std::endl;
    for (const auto& pair : statsBefore) {
        std::cout << "  " << pair.first << ": " << pair.second << std::endl;
    }
    
    // Add a new server
    std::cout << "\nAdding server4..." << std::endl;
    store.addServer("server4");
    std::cout << "Servers after addition: " << store.getServerCount() << std::endl;
    
    // Note: In a real distributed system, keys would be rebalanced
    // Here we just show that new keys will be distributed to the new server
    std::cout << "\nAdding 50 new keys after server addition..." << std::endl;
    for (int i = 100; i < 150; ++i) {
        std::string key = "key_" + std::to_string(i);
        store.set(key, "value_" + std::to_string(i));
    }
    
    auto statsAfter = store.getStats();
    std::cout << "\nKeys per server after adding new keys:" << std::endl;
    for (const auto& pair : statsAfter) {
        std::cout << "  " << pair.first << ": " << pair.second << std::endl;
    }
    std::cout << std::endl;
}

void testServerRemoval() {
    std::cout << "=== Server Removal Test ===" << std::endl;
    
    KeyValueStore store;
    
    store.addServer("server1");
    store.addServer("server2");
    store.addServer("server3");
    
    // Store keys
    for (int i = 0; i < 200; ++i) {
        std::string key = "key_" + std::to_string(i);
        store.set(key, "value_" + std::to_string(i));
    }
    
    auto statsBefore = store.getStats();
    std::cout << "Keys per server before removal:" << std::endl;
    for (const auto& pair : statsBefore) {
        std::cout << "  " << pair.first << ": " << pair.second << std::endl;
    }
    
    // Remove a server
    std::cout << "\nRemoving server2..." << std::endl;
    bool removed = store.removeServer("server2");
    std::cout << "Removal successful: " << (removed ? "Yes" : "No") << std::endl;
    std::cout << "Servers after removal: " << store.getServerCount() << std::endl;
    
    auto statsAfter = store.getStats();
    std::cout << "\nKeys per server after removal:" << std::endl;
    for (const auto& pair : statsAfter) {
        std::cout << "  " << pair.first << ": " << pair.second << std::endl;
    }
    
    // Verify keys are still accessible (they should be reassigned)
    std::cout << "\nVerifying key accessibility after server removal..." << std::endl;
    int accessible = 0;
    for (int i = 0; i < 200; ++i) {
        std::string key = "key_" + std::to_string(i);
        if (store.exists(key)) {
            accessible++;
        }
    }
    std::cout << "Accessible keys: " << accessible << " out of 200" << std::endl;
    std::cout << std::endl;
}

void testUpdateOperations() {
    std::cout << "=== Update Operations Test ===" << std::endl;
    
    KeyValueStore store;
    
    store.addServer("server1");
    store.addServer("server2");
    store.addServer("server3");
    
    // Store initial value
    std::string key = "user:1001";
    store.set(key, "John Doe");
    std::cout << "Initial value: " << store.get(key) << std::endl;
    std::string initialServer = store.getServerForKey(key);
    std::cout << "Stored on: " << initialServer << std::endl;
    
    // Update value
    store.set(key, "John Doe Updated");
    std::cout << "Updated value: " << store.get(key) << std::endl;
    std::string updatedServer = store.getServerForKey(key);
    std::cout << "Stored on: " << updatedServer << std::endl;
    
    if (initialServer == updatedServer) {
        std::cout << "Key remains on same server (expected)" << std::endl;
    } else {
        std::cout << "Key moved to different server (possible with hash ring changes)" << std::endl;
    }
    std::cout << std::endl;
}

void testDeleteOperations() {
    std::cout << "=== Delete Operations Test ===" << std::endl;
    
    KeyValueStore store;
    
    store.addServer("server1");
    store.addServer("server2");
    
    // Store some keys
    store.set("key1", "value1");
    store.set("key2", "value2");
    store.set("key3", "value3");
    
    std::cout << "Total entries before deletion: " << store.getTotalEntries() << std::endl;
    
    // Delete a key
    bool deleted = store.remove("key2");
    std::cout << "Deleted key2: " << (deleted ? "Yes" : "No") << std::endl;
    
    std::cout << "Total entries after deletion: " << store.getTotalEntries() << std::endl;
    std::cout << "key2 exists: " << (store.exists("key2") ? "Yes" : "No") << std::endl;
    std::cout << "key1 exists: " << (store.exists("key1") ? "Yes" : "No") << std::endl;
    
    // Try to delete non-existent key
    bool deleted2 = store.remove("nonexistent");
    std::cout << "Deleted nonexistent key: " << (deleted2 ? "Yes" : "No") << std::endl;
    std::cout << std::endl;
}

void testConcurrentAccess() {
    std::cout << "=== Thread Safety Test ===" << std::endl;
    
    KeyValueStore store;
    
    store.addServer("server1");
    store.addServer("server2");
    store.addServer("server3");
    
    std::atomic<int> successCount(0);
    std::atomic<int> failCount(0);
    
    auto writer = [&store, &successCount, &failCount](int threadId) {
        for (int i = 0; i < 100; ++i) {
            std::string key = "thread_" + std::to_string(threadId) + "_key_" + std::to_string(i);
            if (store.set(key, "value_" + std::to_string(i))) {
                successCount++;
            } else {
                failCount++;
            }
        }
    };
    
    auto reader = [&store, &successCount](int threadId) {
        for (int i = 0; i < 100; ++i) {
            std::string key = "thread_" + std::to_string(threadId) + "_key_" + std::to_string(i);
            std::string value = store.get(key);
            if (!value.empty()) {
                successCount++;
            }
        }
    };
    
    std::vector<std::thread> threads;
    const int numThreads = 10;
    
    std::cout << "Starting " << numThreads << " writer threads..." << std::endl;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(writer, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    threads.clear();
    
    std::cout << "Writer operations - Success: " << successCount.load() 
              << ", Failed: " << failCount.load() << std::endl;
    
    successCount = 0;
    std::cout << "\nStarting " << numThreads << " reader threads..." << std::endl;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(reader, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Reader operations - Success: " << successCount.load() << std::endl;
    std::cout << "Total entries: " << store.getTotalEntries() << std::endl;
    std::cout << std::endl;
}

void testServerKeysRetrieval() {
    std::cout << "=== Server Keys Retrieval Test ===" << std::endl;
    
    KeyValueStore store;
    
    store.addServer("server1");
    store.addServer("server2");
    store.addServer("server3");
    
    // Store keys
    for (int i = 0; i < 50; ++i) {
        std::string key = "key_" + std::to_string(i);
        store.set(key, "value_" + std::to_string(i));
    }
    
    // Get keys for each server
    auto servers = store.getServers();
    for (const auto& server : servers) {
        auto keys = store.getKeysForServer(server);
        std::cout << server << " has " << keys.size() << " keys" << std::endl;
        if (keys.size() <= 5) {
            std::cout << "  Keys: ";
            for (const auto& key : keys) {
                std::cout << key << " ";
            }
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}

void testEdgeCases() {
    std::cout << "=== Edge Cases Test ===" << std::endl;
    
    KeyValueStore store;
    
    // Test operations without servers
    std::cout << "Testing operations without servers..." << std::endl;
    bool setResult = store.set("key1", "value1");
    std::cout << "Set without servers: " << (setResult ? "Success" : "Failed (expected)") << std::endl;
    
    // Add servers
    store.addServer("server1");
    
    // Test empty key
    std::cout << "\nTesting empty key..." << std::endl;
    bool emptyKeyResult = store.set("", "value");
    std::cout << "Set with empty key: " << (emptyKeyResult ? "Success" : "Failed (expected)") << std::endl;
    
    // Test duplicate server addition
    std::cout << "\nTesting duplicate server addition..." << std::endl;
    bool duplicate = store.addServer("server1");
    std::cout << "Add duplicate server: " << (duplicate ? "Success" : "Failed (expected)") << std::endl;
    
    // Test removing non-existent server
    std::cout << "\nTesting removal of non-existent server..." << std::endl;
    bool removeResult = store.removeServer("nonexistent");
    std::cout << "Remove non-existent server: " << (removeResult ? "Success" : "Failed (expected)") << std::endl;
    
    std::cout << std::endl;
}

void runAllTests() {
    try {
        testBasicOperations();
        testServerDistribution();
        testKeyServerMapping();
        testServerAddition();
        testServerRemoval();
        testUpdateOperations();
        testDeleteOperations();
        testConcurrentAccess();
        testServerKeysRetrieval();
        testEdgeCases();
        
        std::cout << "========================================" << std::endl;
        std::cout << "All tests completed successfully!" << std::endl;
        std::cout << "========================================" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        throw;
    }
}

