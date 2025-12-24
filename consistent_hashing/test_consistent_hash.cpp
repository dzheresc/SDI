#include "consistent_hash.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <set>
#include <atomic>
#include <cassert>

void testBasicOperations() {
    std::cout << "=== Basic Operations Test ===" << std::endl;
    
    ConsistentHash hashRing(100);
    
    // Add nodes
    std::cout << "Adding nodes: node1, node2, node3..." << std::endl;
    hashRing.addNode("node1");
    hashRing.addNode("node2");
    hashRing.addNode("node3");
    
    std::cout << "Physical nodes: " << hashRing.getNodeCount() << std::endl;
    std::cout << "Virtual nodes: " << hashRing.getVirtualNodeCount() << std::endl;
    std::cout << std::endl;
    
    // Test key lookups
    std::cout << "Testing key lookups:" << std::endl;
    std::vector<std::string> testKeys = {"key1", "key2", "key3", "key4", "key5"};
    
    for (const auto& key : testKeys) {
        std::string node = hashRing.getNode(key);
        std::cout << "Key '" << key << "' -> Node: " << node << std::endl;
    }
    std::cout << std::endl;
}

void testNodeRemoval() {
    std::cout << "=== Node Removal Test ===" << std::endl;
    
    ConsistentHash hashRing(100);
    
    hashRing.addNode("node1");
    hashRing.addNode("node2");
    hashRing.addNode("node3");
    
    std::cout << "Nodes before removal: " << hashRing.getNodeCount() << std::endl;
    
    // Test key assignment before removal
    std::string testKey = "test_key_123";
    std::string originalNode = hashRing.getNode(testKey);
    std::cout << "Key '" << testKey << "' originally assigned to: " << originalNode << std::endl;
    
    // Remove a node
    std::cout << "\nRemoving node2..." << std::endl;
    bool removed = hashRing.removeNode("node2");
    std::cout << "Removal successful: " << (removed ? "Yes" : "No") << std::endl;
    std::cout << "Nodes after removal: " << hashRing.getNodeCount() << std::endl;
    
    // Check if key is reassigned
    std::string newNode = hashRing.getNode(testKey);
    std::cout << "Key '" << testKey << "' now assigned to: " << newNode << std::endl;
    
    if (originalNode == "node2") {
        std::cout << "Key was reassigned (expected since original node was removed)" << std::endl;
    } else {
        std::cout << "Key assignment unchanged (expected if original node still exists)" << std::endl;
    }
    std::cout << std::endl;
}

void testNodeAddition() {
    std::cout << "=== Node Addition Test ===" << std::endl;
    
    ConsistentHash hashRing(100);
    
    hashRing.addNode("node1");
    hashRing.addNode("node2");
    
    std::cout << "Initial nodes: " << hashRing.getNodeCount() << std::endl;
    
    // Test key assignment before addition
    std::string testKey = "test_key_456";
    std::string originalNode = hashRing.getNode(testKey);
    std::cout << "Key '" << testKey << "' assigned to: " << originalNode << std::endl;
    
    // Add a new node
    std::cout << "\nAdding node3..." << std::endl;
    hashRing.addNode("node3");
    std::cout << "Nodes after addition: " << hashRing.getNodeCount() << std::endl;
    
    // Check if key is reassigned
    std::string newNode = hashRing.getNode(testKey);
    std::cout << "Key '" << testKey << "' now assigned to: " << newNode << std::endl;
    
    if (originalNode != newNode) {
        std::cout << "Key was reassigned to new node" << std::endl;
    } else {
        std::cout << "Key assignment unchanged (minimal remapping)" << std::endl;
    }
    std::cout << std::endl;
}

void testDistribution() {
    std::cout << "=== Distribution Test ===" << std::endl;
    
    ConsistentHash hashRing(150);
    
    hashRing.addNode("node1");
    hashRing.addNode("node2");
    hashRing.addNode("node3");
    hashRing.addNode("node4");
    
    std::cout << "Testing key distribution with 4 nodes..." << std::endl;
    
    auto stats = hashRing.getDistributionStats(10000);
    
    std::cout << "\nDistribution statistics:" << std::endl;
    size_t total = 0;
    for (const auto& pair : stats) {
        std::cout << "  " << pair.first << ": " << pair.second << " keys ("
                  << std::fixed << std::setprecision(2)
                  << (100.0 * pair.second / 10000) << "%)" << std::endl;
        total += pair.second;
    }
    std::cout << "  Total: " << total << " keys" << std::endl;
    std::cout << std::endl;
}

void testReplication() {
    std::cout << "=== Replication Test ===" << std::endl;
    
    ConsistentHash hashRing(100);
    
    hashRing.addNode("node1");
    hashRing.addNode("node2");
    hashRing.addNode("node3");
    hashRing.addNode("node4");
    
    std::string testKey = "replicated_key";
    
    std::cout << "Getting 3 replicas for key '" << testKey << "':" << std::endl;
    auto nodes = hashRing.getNodes(testKey, 3);
    
    for (size_t i = 0; i < nodes.size(); ++i) {
        std::cout << "  Replica " << (i + 1) << ": " << nodes[i] << std::endl;
    }
    
    // Verify all nodes are unique
    std::set<std::string> uniqueNodes(nodes.begin(), nodes.end());
    if (uniqueNodes.size() == nodes.size()) {
        std::cout << "All replicas are unique (correct)" << std::endl;
    } else {
        std::cout << "Warning: Some replicas are duplicates" << std::endl;
    }
    std::cout << std::endl;
}

void testConsistency() {
    std::cout << "=== Consistency Test ===" << std::endl;
    
    ConsistentHash hashRing(100);
    
    hashRing.addNode("node1");
    hashRing.addNode("node2");
    hashRing.addNode("node3");
    
    std::cout << "Testing that same key always maps to same node..." << std::endl;
    
    std::string testKey = "consistent_key";
    std::string firstNode = hashRing.getNode(testKey);
    
    bool consistent = true;
    for (int i = 0; i < 100; ++i) {
        std::string node = hashRing.getNode(testKey);
        if (node != firstNode) {
            consistent = false;
            break;
        }
    }
    
    if (consistent) {
        std::cout << "Consistency check passed: Key always maps to same node" << std::endl;
    } else {
        std::cout << "Consistency check failed: Key maps to different nodes" << std::endl;
    }
    std::cout << std::endl;
}

void testMinimalRemapping() {
    std::cout << "=== Minimal Remapping Test ===" << std::endl;
    
    ConsistentHash hashRing(150);
    
    hashRing.addNode("node1");
    hashRing.addNode("node2");
    hashRing.addNode("node3");
    
    // Test 1000 keys before adding a node
    std::vector<std::string> testKeys;
    std::map<std::string, std::string> beforeMapping;
    
    for (int i = 0; i < 1000; ++i) {
        std::string key = "key_" + std::to_string(i);
        testKeys.push_back(key);
        beforeMapping[key] = hashRing.getNode(key);
    }
    
    // Add a new node
    std::cout << "Adding node4..." << std::endl;
    hashRing.addNode("node4");
    
    // Check how many keys were remapped
    int remapped = 0;
    for (const auto& key : testKeys) {
        std::string afterNode = hashRing.getNode(key);
        if (beforeMapping[key] != afterNode) {
            remapped++;
        }
    }
    
    double remapPercentage = (100.0 * remapped) / 1000;
    std::cout << "Keys remapped: " << remapped << " out of 1000 (" 
              << std::fixed << std::setprecision(2) << remapPercentage << "%)" << std::endl;
    std::cout << "Expected: ~25% (1 new node out of 4 total)" << std::endl;
    std::cout << std::endl;
}

void testConcurrentAccess() {
    std::cout << "=== Thread Safety Test ===" << std::endl;
    
    ConsistentHash hashRing(100);
    
    hashRing.addNode("node1");
    hashRing.addNode("node2");
    hashRing.addNode("node3");
    
    std::atomic<int> successCount(0);
    std::atomic<int> failCount(0);
    
    auto worker = [&hashRing, &successCount, &failCount](int id) {
        for (int i = 0; i < 100; ++i) {
            std::string key = "thread_" + std::to_string(id) + "_key_" + std::to_string(i);
            std::string node = hashRing.getNode(key);
            if (!node.empty()) {
                successCount++;
            } else {
                failCount++;
            }
        }
    };
    
    std::vector<std::thread> threads;
    const int numThreads = 10;
    
    std::cout << "Starting " << numThreads << " threads, each making 100 lookups..." << std::endl;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Successful lookups: " << successCount.load() << std::endl;
    std::cout << "Failed lookups: " << failCount.load() << std::endl;
    std::cout << std::endl;
}

void testVirtualNodes() {
    std::cout << "=== Virtual Nodes Test ===" << std::endl;
    
    std::cout << "Testing with 50 virtual nodes per physical node..." << std::endl;
    ConsistentHash hashRing1(50);
    hashRing1.addNode("node1");
    hashRing1.addNode("node2");
    hashRing1.addNode("node3");
    
    std::cout << "Virtual nodes: " << hashRing1.getVirtualNodeCount() << std::endl;
    
    std::cout << "\nTesting with 200 virtual nodes per physical node..." << std::endl;
    ConsistentHash hashRing2(200);
    hashRing2.addNode("node1");
    hashRing2.addNode("node2");
    hashRing2.addNode("node3");
    
    std::cout << "Virtual nodes: " << hashRing2.getVirtualNodeCount() << std::endl;
    
    std::cout << "\nMore virtual nodes = better distribution but more memory" << std::endl;
    std::cout << std::endl;
}

void testEdgeCases() {
    std::cout << "=== Edge Cases Test ===" << std::endl;
    
    ConsistentHash hashRing(100);
    
    // Test with no nodes
    std::cout << "Testing lookup with no nodes..." << std::endl;
    std::string node = hashRing.getNode("test_key");
    if (node.empty()) {
        std::cout << "Correctly returns empty string when no nodes exist" << std::endl;
    }
    
    // Test adding duplicate node
    std::cout << "\nTesting adding duplicate node..." << std::endl;
    hashRing.addNode("node1");
    size_t count1 = hashRing.getNodeCount();
    hashRing.addNode("node1");  // Duplicate
    size_t count2 = hashRing.getNodeCount();
    if (count1 == count2) {
        std::cout << "Correctly handles duplicate node addition" << std::endl;
    }
    
    // Test removing non-existent node
    std::cout << "\nTesting removal of non-existent node..." << std::endl;
    bool removed = hashRing.removeNode("nonexistent");
    if (!removed) {
        std::cout << "Correctly returns false for non-existent node" << std::endl;
    }
    
    std::cout << std::endl;
}

void runAllTests() {
    try {
        testBasicOperations();
        testNodeRemoval();
        testNodeAddition();
        testDistribution();
        testReplication();
        testConsistency();
        testMinimalRemapping();
        testConcurrentAccess();
        testVirtualNodes();
        testEdgeCases();
        
        std::cout << "========================================" << std::endl;
        std::cout << "All tests completed successfully!" << std::endl;
        std::cout << "========================================" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        throw;
    }
}

