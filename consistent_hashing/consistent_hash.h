#ifndef CONSISTENT_HASH_H
#define CONSISTENT_HASH_H

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <functional>

/**
 * Consistent Hash Ring
 * 
 * Implements consistent hashing algorithm for distributed systems:
 * - Maps keys to nodes using a hash ring
 * - Minimal key remapping when nodes are added/removed
 * - Supports virtual nodes for better distribution
 * - Thread-safe operations
 */
class ConsistentHash {
public:
    /**
     * Constructor
     * @param virtualNodesPerNode Number of virtual nodes per physical node (default: 150)
     */
    explicit ConsistentHash(int virtualNodesPerNode = 150);
    
    /**
     * Add a node to the hash ring
     * @param nodeName Name/identifier of the node
     */
    void addNode(const std::string& nodeName);
    
    /**
     * Remove a node from the hash ring
     * @param nodeName Name/identifier of the node to remove
     * @return true if node was found and removed, false otherwise
     */
    bool removeNode(const std::string& nodeName);
    
    /**
     * Get the node responsible for a given key
     * @param key The key to look up
     * @return Name of the node responsible for the key, or empty string if no nodes exist
     */
    std::string getNode(const std::string& key) const;
    
    /**
     * Get multiple nodes responsible for a key (for replication)
     * @param key The key to look up
     * @param count Number of nodes to return
     * @return Vector of node names
     */
    std::vector<std::string> getNodes(const std::string& key, int count) const;
    
    /**
     * Get the number of physical nodes in the ring
     * @return Number of nodes
     */
    size_t getNodeCount() const;
    
    /**
     * Get the number of virtual nodes in the ring
     * @return Number of virtual nodes
     */
    size_t getVirtualNodeCount() const;
    
    /**
     * Check if a node exists in the ring
     * @param nodeName Name of the node to check
     * @return true if node exists, false otherwise
     */
    bool hasNode(const std::string& nodeName) const;
    
    /**
     * Get all node names in the ring
     * @return Vector of all node names
     */
    std::vector<std::string> getAllNodes() const;
    
    /**
     * Clear all nodes from the ring
     */
    void clear();
    
    /**
     * Get statistics about key distribution
     * @param numTestKeys Number of test keys to use for statistics
     * @return Map of node name to number of keys assigned
     */
    std::map<std::string, size_t> getDistributionStats(size_t numTestKeys = 10000) const;

private:
    int virtualNodesPerNode_;  // Number of virtual nodes per physical node
    std::map<uint32_t, std::string> ring_;  // Hash ring: hash -> node name
    std::map<std::string, std::vector<uint32_t>> nodeToHashes_;  // Node -> list of hash values
    mutable std::mutex mutex_;  // Mutex for thread safety
    
    /**
     * Generate hash value for a string
     * @param input Input string
     * @return 32-bit hash value
     */
    static uint32_t hash(const std::string& input);
    
    /**
     * Generate virtual node name
     * @param nodeName Physical node name
     * @param replicaIndex Virtual node index
     * @return Virtual node name
     */
    static std::string getVirtualNodeName(const std::string& nodeName, int replicaIndex);
};

#endif // CONSISTENT_HASH_H

