#include "consistent_hash.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <stdexcept>

// FNV-1a hash function (32-bit)
uint32_t ConsistentHash::hash(const std::string& input) {
    const uint32_t FNV_OFFSET_BASIS = 2166136261u;
    const uint32_t FNV_PRIME = 16777619u;
    
    uint32_t hash = FNV_OFFSET_BASIS;
    for (char c : input) {
        hash ^= static_cast<unsigned char>(c);
        hash *= FNV_PRIME;
    }
    return hash;
}

std::string ConsistentHash::getVirtualNodeName(const std::string& nodeName, int replicaIndex) {
    std::ostringstream oss;
    oss << nodeName << "#" << replicaIndex;
    return oss.str();
}

ConsistentHash::ConsistentHash(int virtualNodesPerNode)
    : virtualNodesPerNode_(virtualNodesPerNode)
{
    if (virtualNodesPerNode <= 0) {
        throw std::invalid_argument("Virtual nodes per node must be positive");
    }
}

void ConsistentHash::addNode(const std::string& nodeName) {
    if (nodeName.empty()) {
        throw std::invalid_argument("Node name cannot be empty");
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if node already exists
    if (nodeToHashes_.find(nodeName) != nodeToHashes_.end()) {
        return;  // Node already exists
    }
    
    std::vector<uint32_t> hashes;
    hashes.reserve(virtualNodesPerNode_);
    
    // Add virtual nodes for this physical node
    for (int i = 0; i < virtualNodesPerNode_; ++i) {
        std::string virtualNodeName = getVirtualNodeName(nodeName, i);
        uint32_t hashValue = hash(virtualNodeName);
        
        // Handle hash collisions (very rare, but possible)
        while (ring_.find(hashValue) != ring_.end()) {
            hashValue++;  // Simple collision resolution
        }
        
        ring_[hashValue] = nodeName;
        hashes.push_back(hashValue);
    }
    
    nodeToHashes_[nodeName] = std::move(hashes);
}

bool ConsistentHash::removeNode(const std::string& nodeName) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = nodeToHashes_.find(nodeName);
    if (it == nodeToHashes_.end()) {
        return false;  // Node doesn't exist
    }
    
    // Remove all virtual nodes from the ring
    for (uint32_t hashValue : it->second) {
        ring_.erase(hashValue);
    }
    
    // Remove node from mapping
    nodeToHashes_.erase(it);
    
    return true;
}

std::string ConsistentHash::getNode(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (ring_.empty()) {
        return "";
    }
    
    uint32_t keyHash = hash(key);
    
    // Find the first node with hash >= keyHash (clockwise search)
    auto it = ring_.lower_bound(keyHash);
    
    // If no node found, wrap around to the beginning (circular ring)
    if (it == ring_.end()) {
        it = ring_.begin();
    }
    
    return it->second;
}

std::vector<std::string> ConsistentHash::getNodes(const std::string& key, int count) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> nodes;
    if (ring_.empty() || count <= 0) {
        return nodes;
    }
    
    uint32_t keyHash = hash(key);
    
    // Find starting position
    auto it = ring_.lower_bound(keyHash);
    if (it == ring_.end()) {
        it = ring_.begin();
    }
    
    // Collect unique nodes
    std::map<std::string, bool> seen;
    auto startIt = it;
    
    do {
        const std::string& nodeName = it->second;
        if (seen.find(nodeName) == seen.end()) {
            nodes.push_back(nodeName);
            seen[nodeName] = true;
            
            if (static_cast<int>(nodes.size()) >= count) {
                break;
            }
        }
        
        ++it;
        if (it == ring_.end()) {
            it = ring_.begin();
        }
    } while (it != startIt && static_cast<int>(nodes.size()) < count);
    
    return nodes;
}

size_t ConsistentHash::getNodeCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return nodeToHashes_.size();
}

size_t ConsistentHash::getVirtualNodeCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return ring_.size();
}

bool ConsistentHash::hasNode(const std::string& nodeName) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return nodeToHashes_.find(nodeName) != nodeToHashes_.end();
}

std::vector<std::string> ConsistentHash::getAllNodes() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> nodes;
    nodes.reserve(nodeToHashes_.size());
    
    for (const auto& pair : nodeToHashes_) {
        nodes.push_back(pair.first);
    }
    
    return nodes;
}

void ConsistentHash::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    ring_.clear();
    nodeToHashes_.clear();
}

std::map<std::string, size_t> ConsistentHash::getDistributionStats(size_t numTestKeys) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::map<std::string, size_t> stats;
    
    if (ring_.empty()) {
        return stats;
    }
    
    // Initialize stats for all nodes
    for (const auto& pair : nodeToHashes_) {
        stats[pair.first] = 0;
    }
    
    // Generate test keys and count distribution
    for (size_t i = 0; i < numTestKeys; ++i) {
        std::string testKey = "key_" + std::to_string(i);
        uint32_t keyHash = hash(testKey);
        
        auto it = ring_.lower_bound(keyHash);
        if (it == ring_.end()) {
            it = ring_.begin();
        }
        
        stats[it->second]++;
    }
    
    return stats;
}

