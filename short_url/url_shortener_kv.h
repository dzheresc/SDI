#ifndef URL_SHORTENER_KV_H
#define URL_SHORTENER_KV_H

#include <string>
#include <fstream>
#include <stdexcept>
#include <cstdint>
#include <memory>
#include <vector>

// Forward declaration
class KeyValueStore;

/**
 * URL Shortener Service with KeyValue Store Backend
 * 
 * Alternative implementation using KeyValueStore for distributed storage.
 * Provides URL shortening functionality with:
 * - Base62 encoding for short names
 * - KeyValueStore backend for distributed/horizontal scaling
 * - CSV file persistence (save/load)
 */
class UrlShortenerKV {
public:
    /**
     * Constructor
     * @param baseUrl Base URL for shortened links (e.g., "https://short.ly/")
     * @param virtualNodesPerNode Number of virtual nodes per server for consistent hashing (default: 150)
     */
    explicit UrlShortenerKV(
        const std::string& baseUrl = "https://short.ly/",
        int virtualNodesPerNode = 150
    );
    
    /**
     * Destructor
     */
    ~UrlShortenerKV();
    
    // Non-copyable, movable
    UrlShortenerKV(const UrlShortenerKV&) = delete;
    UrlShortenerKV& operator=(const UrlShortenerKV&) = delete;
    UrlShortenerKV(UrlShortenerKV&&) = default;
    UrlShortenerKV& operator=(UrlShortenerKV&&) = default;
    
    /**
     * Shorten a URL
     * @param longUrl The original long URL to shorten
     * @return Shortened URL (baseUrl + short code)
     */
    std::string shorten(const std::string& longUrl);
    
    /**
     * Expand a short code to the original URL
     * @param shortCode The short code (without base URL)
     * @return Original long URL, or empty string if not found
     */
    std::string expand(const std::string& shortCode);
    
    /**
     * Expand a full shortened URL to the original URL
     * @param shortUrl The full shortened URL
     * @return Original long URL, or empty string if not found
     */
    std::string expandUrl(const std::string& shortUrl);
    
    /**
     * Check if a short code exists
     * @param shortCode The short code to check
     * @return true if code exists, false otherwise
     */
    bool exists(const std::string& shortCode) const;
    
    /**
     * Get the number of shortened URLs
     * @return Number of URLs in the database
     */
    size_t size() const;
    
    /**
     * Check if the database is empty
     * @return true if empty, false otherwise
     */
    bool empty() const;
    
    /**
     * Clear all shortened URLs
     */
    void clear();
    
    /**
     * Save the database to a CSV file
     * @param filename Path to the CSV file
     * @return true if successful, false otherwise
     */
    bool saveToFile(const std::string& filename) const;
    
    /**
     * Load the database from a CSV file
     * @param filename Path to the CSV file
     * @return true if successful, false otherwise
     */
    bool loadFromFile(const std::string& filename);
    
    /**
     * Get statistics about the database
     * @param totalUrls Output: total number of URLs
     * @param totalShortCodes Output: total number of short codes
     */
    void getStats(size_t& totalUrls, size_t& totalShortCodes) const;
    
    /**
     * Add a server to the KeyValue store cluster
     * @param serverId Server identifier
     * @return true if added, false otherwise
     */
    bool addServer(const std::string& serverId);
    
    /**
     * Remove a server from the cluster
     * @param serverId Server identifier
     * @return true if removed, false otherwise
     */
    bool removeServer(const std::string& serverId);
    
    /**
     * Get all servers in the cluster
     * @return Vector of server identifiers
     */
    std::vector<std::string> getServers() const;
    
    /**
     * Get the server responsible for a given short code
     * @param shortCode The short code
     * @return Server identifier, or empty string if no servers
     */
    std::string getServerForKey(const std::string& shortCode) const;
    
    /**
     * Encode a number to base62 (public for testing/utility)
     * @param num Number to encode
     * @return Base62 encoded string
     */
    static std::string encodeBase62(uint64_t num);
    
    /**
     * Decode a base62 string to a number (public for testing/utility)
     * @param encoded Base62 encoded string
     * @return Decoded number
     */
    static uint64_t decodeBase62(const std::string& encoded);

private:
    std::string baseUrl_;                    // Base URL for shortened links
    std::unique_ptr<KeyValueStore> kvStore_; // KeyValue store backend
    std::unique_ptr<KeyValueStore> reverseKvStore_; // Reverse mapping: longUrl -> shortCode
    uint64_t nextId_;                        // Next ID to use for encoding
    std::vector<std::string> shortCodeIndex_; // Index of all short codes for iteration
    
    // Key prefixes for different data types
    static constexpr const char* SHORT_CODE_PREFIX = "sc:";
    static constexpr const char* LONG_URL_PREFIX = "url:";
    static constexpr const char* NEXT_ID_KEY = "next_id";
    static constexpr const char* INDEX_KEY = "index";
    
    /**
     * Generate a unique short code
     * @return Unique short code
     */
    std::string generateShortCode();
    
    /**
     * Extract short code from a full shortened URL
     * @param shortUrl Full shortened URL
     * @return Short code, or empty string if invalid
     */
    std::string extractShortCode(const std::string& shortUrl) const;
    
    /**
     * Get next ID from store or generate new one
     */
    uint64_t getNextId();
    
    /**
     * Save next ID to store
     */
    void saveNextId(uint64_t id);
    
    /**
     * Save index to store
     */
    void saveIndex();
    
    /**
     * Load index from store
     */
    void loadIndex();
};

#endif // URL_SHORTENER_KV_H

