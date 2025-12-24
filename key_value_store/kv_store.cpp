#include "kv_store.h"
#include "../consistent_hashing/consistent_hash.h"
#include <algorithm>
#include <stdexcept>

KeyValueStore::KeyValueStore(int virtualNodesPerNode)
    : hashRing_(std::make_unique<ConsistentHash>(virtualNodesPerNode))
{
}

KeyValueStore::~KeyValueStore() = default;

bool KeyValueStore::addServer(const std::string& serverId) {
    if (serverId.empty()) {
        throw std::invalid_argument("Server ID cannot be empty");
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if server already exists
    if (serverKeys_.find(serverId) != serverKeys_.end()) {
        return false;
    }
    
    // Add server to hash ring
    hashRing_->addNode(serverId);
    
    // Initialize empty key list for this server
    serverKeys_[serverId] = std::vector<std::string>();
    
    return true;
}

bool KeyValueStore::removeServer(const std::string& serverId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = serverKeys_.find(serverId);
    if (it == serverKeys_.end()) {
        return false;
    }
    
    // Get all keys that were on this server
    std::vector<std::string> keysToReassign = it->second;
    
    // Remove server from hash ring
    hashRing_->removeNode(serverId);
    
    // Remove server from tracking
    serverKeys_.erase(it);
    
    // Reassign keys to other servers (in a real implementation, this would trigger data migration)
    for (const auto& key : keysToReassign) {
        if (data_.find(key) != data_.end()) {
            std::string newServer = hashRing_->getNode(key);
            if (!newServer.empty()) {
                updateServerKeys(key, newServer);
            }
        }
    }
    
    return true;
}

bool KeyValueStore::set(const std::string& key, const std::string& value) {
    if (key.empty()) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (hashRing_->getNodeCount() == 0) {
        return false;  // No servers available
    }
    
    // Determine which server should store this key
    std::string serverId = hashRing_->getNode(key);
    if (serverId.empty()) {
        return false;
    }
    
    // Check if key already exists on a different server
    bool keyExists = (data_.find(key) != data_.end());
    if (keyExists) {
        // Find old server and remove key from its tracking
        for (auto& pair : serverKeys_) {
            auto& keys = pair.second;
            auto keyIt = std::find(keys.begin(), keys.end(), key);
            if (keyIt != keys.end()) {
                keys.erase(keyIt);
                break;
            }
        }
    }
    
    // Store the key-value pair
    data_[key] = value;
    
    // Update server key tracking
    updateServerKeys(key, serverId);
    
    return true;
}

std::string KeyValueStore::get(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = data_.find(key);
    if (it != data_.end()) {
        return it->second;
    }
    
    return "";
}

bool KeyValueStore::remove(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = data_.find(key);
    if (it == data_.end()) {
        return false;
    }
    
    // Find which server has this key
    std::string serverId = hashRing_->getNode(key);
    if (!serverId.empty()) {
        removeFromServerKeys(key, serverId);
    }
    
    // Remove the key-value pair
    data_.erase(it);
    
    return true;
}

bool KeyValueStore::exists(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return data_.find(key) != data_.end();
}

std::vector<std::string> KeyValueStore::getKeysForServer(const std::string& serverId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = serverKeys_.find(serverId);
    if (it != serverKeys_.end()) {
        return it->second;
    }
    
    return std::vector<std::string>();
}

std::vector<std::string> KeyValueStore::getServers() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> servers;
    servers.reserve(serverKeys_.size());
    
    for (const auto& pair : serverKeys_) {
        servers.push_back(pair.first);
    }
    
    return servers;
}

std::string KeyValueStore::getServerForKey(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return hashRing_->getNode(key);
}

std::map<std::string, size_t> KeyValueStore::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::map<std::string, size_t> stats;
    
    for (const auto& pair : serverKeys_) {
        stats[pair.first] = pair.second.size();
    }
    
    return stats;
}

void KeyValueStore::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    data_.clear();
    serverKeys_.clear();
    hashRing_->clear();
}

size_t KeyValueStore::getServerCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return hashRing_->getNodeCount();
}

size_t KeyValueStore::getTotalEntries() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return data_.size();
}

void KeyValueStore::updateServerKeys(const std::string& key, const std::string& serverId) {
    auto it = serverKeys_.find(serverId);
    if (it != serverKeys_.end()) {
        auto& keys = it->second;
        // Check if key is not already in the list
        if (std::find(keys.begin(), keys.end(), key) == keys.end()) {
            keys.push_back(key);
        }
    }
}

void KeyValueStore::removeFromServerKeys(const std::string& key, const std::string& serverId) {
    auto it = serverKeys_.find(serverId);
    if (it != serverKeys_.end()) {
        auto& keys = it->second;
        keys.erase(std::remove(keys.begin(), keys.end(), key), keys.end());
    }
}

