#ifndef KV_STORE_H
#define KV_STORE_H

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <memory>
#include <functional>

// Forward declaration
class ConsistentHash;

/**
 * Distributed Key-Value Store
 * 
 * A distributed key-value store that uses consistent hashing for
 * horizontal scaling and distribution across multiple servers.
 */
class KeyValueStore {
public:
    /**
     * Constructor
     * @param virtualNodesPerNode Number of virtual nodes per server (default: 150)
     */
    explicit KeyValueStore(int virtualNodesPerNode = 150);
    
    /**
     * Destructor
     */
    ~KeyValueStore();
    
    /**
     * Add a server to the cluster
     * @param serverId Unique identifier for the server
     * @return true if server was added, false if it already exists
     */
    bool addServer(const std::string& serverId);
    
    /**
     * Remove a server from the cluster
     * @param serverId Identifier of the server to remove
     * @return true if server was removed, false if it doesn't exist
     */
    bool removeServer(const std::string& serverId);
    
    /**
     * Store a key-value pair
     * @param key The key
     * @param value The value
     * @return true if successful, false otherwise
     */
    bool set(const std::string& key, const std::string& value);
    
    /**
     * Retrieve a value by key
     * @param key The key to look up
     * @return The value if found, empty string otherwise
     */
    std::string get(const std::string& key) const;
    
    /**
     * Delete a key-value pair
     * @param key The key to delete
     * @return true if key was found and deleted, false otherwise
     */
    bool remove(const std::string& key);
    
    /**
     * Check if a key exists
     * @param key The key to check
     * @return true if key exists, false otherwise
     */
    bool exists(const std::string& key) const;
    
    /**
     * Get all keys stored on a specific server
     * @param serverId The server identifier
     * @return Vector of keys stored on that server
     */
    std::vector<std::string> getKeysForServer(const std::string& serverId) const;
    
    /**
     * Get all servers in the cluster
     * @return Vector of server identifiers
     */
    std::vector<std::string> getServers() const;
    
    /**
     * Get the server responsible for a given key
     * @param key The key to check
     * @return Server identifier, or empty string if no servers exist
     */
    std::string getServerForKey(const std::string& key) const;
    
    /**
     * Get statistics about the store
     * @return Map of server ID to number of keys stored
     */
    std::map<std::string, size_t> getStats() const;
    
    /**
     * Clear all data from the store
     */
    void clear();
    
    /**
     * Get the number of servers in the cluster
     * @return Number of servers
     */
    size_t getServerCount() const;
    
    /**
     * Get the total number of key-value pairs
     * @return Total number of entries
     */
    size_t getTotalEntries() const;

private:
    std::unique_ptr<ConsistentHash> hashRing_;  // Consistent hash ring for server selection
    std::map<std::string, std::string> data_;    // Key-value storage (simplified - in real implementation, this would be distributed)
    std::map<std::string, std::vector<std::string>> serverKeys_;  // Track which keys belong to which server
    mutable std::mutex mutex_;  // Mutex for thread safety
    
    /**
     * Internal method to update server key tracking
     */
    void updateServerKeys(const std::string& key, const std::string& serverId);
    
    /**
     * Internal method to remove key from server tracking
     */
    void removeFromServerKeys(const std::string& key, const std::string& serverId);
};

#endif // KV_STORE_H

