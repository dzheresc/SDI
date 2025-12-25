#include "url_shortener.h"
#include <sstream>
#include <algorithm>
#include <cctype>

// Base62 character set: 0-9, a-z, A-Z
static const char BASE62_CHARS[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const int BASE62 = 62;

UrlShortener::UrlShortener(const std::string& baseUrl)
    : baseUrl_(baseUrl)
    , nextId_(1)  // Start from 1, 0 encodes to "0"
{
    if (baseUrl.empty()) {
        throw std::invalid_argument("Base URL cannot be empty");
    }
}

std::string UrlShortener::shorten(const std::string& longUrl) {
    if (longUrl.empty()) {
        throw std::invalid_argument("Long URL cannot be empty");
    }
    
    // Check if URL already exists
    auto it = reverseMap_.find(longUrl);
    if (it != reverseMap_.end()) {
        // URL already shortened, return existing short URL
        return baseUrl_ + it->second;
    }
    
    // Generate new short code
    std::string shortCode = generateShortCode();
    
    // Store mapping
    urlMap_[shortCode] = longUrl;
    reverseMap_[longUrl] = shortCode;
    
    return baseUrl_ + shortCode;
}

std::string UrlShortener::expand(const std::string& shortCode) {
    if (shortCode.empty()) {
        return "";
    }
    
    auto it = urlMap_.find(shortCode);
    if (it != urlMap_.end()) {
        return it->second;
    }
    
    return "";
}

std::string UrlShortener::expandUrl(const std::string& shortUrl) {
    std::string shortCode = extractShortCode(shortUrl);
    if (shortCode.empty()) {
        return "";
    }
    
    return expand(shortCode);
}

bool UrlShortener::exists(const std::string& shortCode) const {
    return urlMap_.find(shortCode) != urlMap_.end();
}

size_t UrlShortener::size() const {
    return urlMap_.size();
}

bool UrlShortener::empty() const {
    return urlMap_.empty();
}

void UrlShortener::clear() {
    urlMap_.clear();
    reverseMap_.clear();
    nextId_ = 1;
}

bool UrlShortener::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Write CSV header
    file << "short_code,long_url\n";
    
    // Write all mappings
    for (const auto& pair : urlMap_) {
        file << pair.first << "," << pair.second << "\n";
    }
    
    file.close();
    return file.good();
}

bool UrlShortener::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Clear existing data
    clear();
    
    std::string line;
    bool firstLine = true;
    
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
        
        // Store mapping
        urlMap_[shortCode] = longUrl;
        reverseMap_[longUrl] = shortCode;
        
        // Update nextId_ based on decoded short code
        try {
            uint64_t decodedId = decodeBase62(shortCode);
            if (decodedId >= nextId_) {
                nextId_ = decodedId + 1;
            }
        } catch (...) {
            // If decoding fails, continue with current nextId_
        }
    }
    
    file.close();
    return true;
}

void UrlShortener::getStats(size_t& totalUrls, size_t& totalShortCodes) const {
    totalUrls = reverseMap_.size();
    totalShortCodes = urlMap_.size();
}

std::string UrlShortener::encodeBase62(uint64_t num) {
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

uint64_t UrlShortener::decodeBase62(const std::string& encoded) {
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

std::string UrlShortener::generateShortCode() {
    // Generate short code from nextId_
    std::string shortCode = encodeBase62(nextId_);
    
    // Ensure uniqueness (shouldn't happen with sequential IDs, but safety check)
    while (urlMap_.find(shortCode) != urlMap_.end()) {
        nextId_++;
        shortCode = encodeBase62(nextId_);
    }
    
    nextId_++;
    return shortCode;
}

std::string UrlShortener::extractShortCode(const std::string& shortUrl) const {
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

