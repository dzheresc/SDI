#include "url_shortener_kv.h"
#include "../key_value_store/kv_store.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <set>

// Base62 character set: 0-9, a-z, A-Z
static const char BASE62_CHARS[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const int BASE62 = 62;

UrlShortenerKV::UrlShortenerKV(const std::string& baseUrl, int virtualNodesPerNode)
    : baseUrl_(baseUrl)
    , kvStore_(std::make_unique<KeyValueStore>(virtualNodesPerNode))
    , reverseKvStore_(std::make_unique<KeyValueStore>(virtualNodesPerNode))
    , nextId_(1)
{
    if (baseUrl.empty()) {
        throw std::invalid_argument("Base URL cannot be empty");
    }
    
    // Add default server for single-node operation
    kvStore_->addServer("server1");
    reverseKvStore_->addServer("server1");
    
    // Load nextId_ from store if it exists
    std::string nextIdStr = kvStore_->get(NEXT_ID_KEY);
    if (!nextIdStr.empty()) {
        try {
            nextId_ = std::stoull(nextIdStr);
        } catch (...) {
            nextId_ = 1;
        }
    }
    
    // Load index from store if it exists
    loadIndex();
}

UrlShortenerKV::~UrlShortenerKV() = default;

std::string UrlShortenerKV::shorten(const std::string& longUrl) {
    if (longUrl.empty()) {
        throw std::invalid_argument("Long URL cannot be empty");
    }
    
    // Check if URL already exists using reverse store
    std::string reverseKey = LONG_URL_PREFIX + longUrl;
    std::string existingCode = reverseKvStore_->get(reverseKey);
    
    if (!existingCode.empty()) {
        // URL already shortened, return existing short URL
        // Note: existingCode should already be in index from initial creation
        return baseUrl_ + existingCode;
    }
    
    // Generate new short code
    std::string shortCode = generateShortCode();
    
    // Store in both stores
    std::string shortCodeKey = SHORT_CODE_PREFIX + shortCode;
    kvStore_->set(shortCodeKey, longUrl);
    reverseKvStore_->set(reverseKey, shortCode);
    
    // Add to index
    shortCodeIndex_.push_back(shortCode);
    saveIndex();
    
    return baseUrl_ + shortCode;
}

std::string UrlShortenerKV::expand(const std::string& shortCode) {
    if (shortCode.empty()) {
        return "";
    }
    
    std::string key = SHORT_CODE_PREFIX + shortCode;
    return kvStore_->get(key);
}

std::string UrlShortenerKV::expandUrl(const std::string& shortUrl) {
    std::string shortCode = extractShortCode(shortUrl);
    if (shortCode.empty()) {
        return "";
    }
    
    return expand(shortCode);
}

bool UrlShortenerKV::exists(const std::string& shortCode) const {
    std::string key = SHORT_CODE_PREFIX + shortCode;
    return kvStore_->exists(key);
}

size_t UrlShortenerKV::size() const {
    return shortCodeIndex_.size();
}

bool UrlShortenerKV::empty() const {
    return size() == 0;
}

void UrlShortenerKV::clear() {
    kvStore_->clear();
    reverseKvStore_->clear();
    shortCodeIndex_.clear();
    nextId_ = 1;
    saveNextId(nextId_);
    saveIndex();
}

bool UrlShortenerKV::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Write CSV header
    file << "short_code,long_url\n";
    
    // Iterate through index and write all mappings
    for (const auto& shortCode : shortCodeIndex_) {
        std::string key = SHORT_CODE_PREFIX + shortCode;
        std::string longUrl = kvStore_->get(key);
        
        if (!longUrl.empty()) {
            file << shortCode << "," << longUrl << "\n";
        }
    }
    
    file.close();
    return file.good();
}

bool UrlShortenerKV::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Clear existing data
    clear();
    
    std::string line;
    bool firstLine = true;
    uint64_t maxId = 0;
    
    while (std::getline(file, line)) {
        // Skip header line
        if (firstLine) {
            firstLine = false;
            continue;
        }
        
        // Skip empty lines
        if (line.empty()) {
            continue;
        }
        
        // Parse CSV line: short_code,long_url
        size_t commaPos = line.find(',');
        if (commaPos == std::string::npos) {
            continue;  // Invalid line
        }
        
        std::string shortCode = line.substr(0, commaPos);
        std::string longUrl = line.substr(commaPos + 1);
        
        // Store in both stores
        std::string shortCodeKey = SHORT_CODE_PREFIX + shortCode;
        std::string reverseKey = LONG_URL_PREFIX + longUrl;
        
        kvStore_->set(shortCodeKey, longUrl);
        reverseKvStore_->set(reverseKey, shortCode);
        
        // Add to index
        shortCodeIndex_.push_back(shortCode);
        
        // Update nextId_ based on decoded short code
        try {
            uint64_t decodedId = decodeBase62(shortCode);
            if (decodedId > maxId) {
                maxId = decodedId;
            }
        } catch (...) {
            // If decoding fails, continue
        }
    }
    
    if (maxId > 0) {
        nextId_ = maxId + 1;
        saveNextId(nextId_);
    }
    
    saveIndex();
    file.close();
    return true;
}

void UrlShortenerKV::getStats(size_t& totalUrls, size_t& totalShortCodes) const {
    totalUrls = reverseKvStore_->getTotalEntries();
    totalShortCodes = kvStore_->getTotalEntries();
}

bool UrlShortenerKV::addServer(const std::string& serverId) {
    bool result1 = kvStore_->addServer(serverId);
    bool result2 = reverseKvStore_->addServer(serverId);
    return result1 && result2;
}

bool UrlShortenerKV::removeServer(const std::string& serverId) {
    bool result1 = kvStore_->removeServer(serverId);
    bool result2 = reverseKvStore_->removeServer(serverId);
    return result1 && result2;
}

std::vector<std::string> UrlShortenerKV::getServers() const {
    return kvStore_->getServers();
}

std::string UrlShortenerKV::getServerForKey(const std::string& shortCode) const {
    std::string key = SHORT_CODE_PREFIX + shortCode;
    return kvStore_->getServerForKey(key);
}

std::string UrlShortenerKV::encodeBase62(uint64_t num) {
    if (num == 0) {
        return "0";
    }
    
    std::string result;
    while (num > 0) {
        result = BASE62_CHARS[num % BASE62] + result;
        num /= BASE62;
    }
    
    return result;
}

uint64_t UrlShortenerKV::decodeBase62(const std::string& encoded) {
    uint64_t result = 0;
    uint64_t base = 1;
    
    for (int i = static_cast<int>(encoded.length()) - 1; i >= 0; --i) {
        char c = encoded[i];
        
        // Find character index in BASE62_CHARS
        int charIndex = -1;
        if (c >= '0' && c <= '9') {
            charIndex = c - '0';
        } else if (c >= 'a' && c <= 'z') {
            charIndex = 10 + (c - 'a');
        } else if (c >= 'A' && c <= 'Z') {
            charIndex = 36 + (c - 'A');
        } else {
            throw std::invalid_argument("Invalid base62 character: " + std::string(1, c));
        }
        
        result += charIndex * base;
        base *= BASE62;
    }
    
    return result;
}

std::string UrlShortenerKV::generateShortCode() {
    // Get next ID
    uint64_t id = getNextId();
    
    // Generate short code from ID
    std::string shortCode = encodeBase62(id);
    
    // Ensure uniqueness (shouldn't happen with sequential IDs, but safety check)
    std::string key = SHORT_CODE_PREFIX + shortCode;
    while (kvStore_->exists(key)) {
        id = getNextId();
        shortCode = encodeBase62(id);
        key = SHORT_CODE_PREFIX + shortCode;
    }
    
    return shortCode;
}

std::string UrlShortenerKV::extractShortCode(const std::string& shortUrl) const {
    // Check if URL starts with baseUrl_
    if (shortUrl.find(baseUrl_) != 0) {
        return "";
    }
    
    // Extract the part after baseUrl_
    std::string shortCode = shortUrl.substr(baseUrl_.length());
    
    // Remove any trailing slashes or whitespace
    while (!shortCode.empty() && (shortCode.back() == '/' || std::isspace(shortCode.back()))) {
        shortCode.pop_back();
    }
    
    return shortCode;
}

uint64_t UrlShortenerKV::getNextId() {
    std::string idStr = kvStore_->get(NEXT_ID_KEY);
    if (!idStr.empty()) {
        try {
            uint64_t id = std::stoull(idStr);
            nextId_ = id + 1;
        } catch (...) {
            // If parsing fails, use current nextId_
        }
    }
    
    uint64_t currentId = nextId_;
    saveNextId(nextId_);
    nextId_++;
    
    return currentId;
}

void UrlShortenerKV::saveNextId(uint64_t id) {
    kvStore_->set(NEXT_ID_KEY, std::to_string(id));
}

void UrlShortenerKV::saveIndex() {
    // Store index as comma-separated list
    std::ostringstream oss;
    for (size_t i = 0; i < shortCodeIndex_.size(); ++i) {
        if (i > 0) {
            oss << ",";
        }
        oss << shortCodeIndex_[i];
    }
    kvStore_->set(INDEX_KEY, oss.str());
}

void UrlShortenerKV::loadIndex() {
    std::string indexStr = kvStore_->get(INDEX_KEY);
    if (indexStr.empty()) {
        return;
    }
    
    shortCodeIndex_.clear();
    std::istringstream iss(indexStr);
    std::string code;
    
    while (std::getline(iss, code, ',')) {
        if (!code.empty()) {
            shortCodeIndex_.push_back(code);
        }
    }
}

