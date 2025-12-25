#ifndef URL_SHORTENER_H
#define URL_SHORTENER_H

#include <string>
#include <unordered_map>
#include <fstream>
#include <stdexcept>
#include <cstdint>

/**
 * URL Shortener Service
 * 
 * Provides URL shortening functionality with:
 * - Base62 encoding for short names
 * - Database-like interface with unordered_map storage
 * - CSV file persistence (save/load)
 */
class UrlShortener {
public:
    /**
     * Constructor
     * @param baseUrl Base URL for shortened links (e.g., "https://short.ly/")
     */
    explicit UrlShortener(const std::string& baseUrl = "https://short.ly/");
    
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
    std::unordered_map<std::string, std::string> urlMap_;  // shortCode -> longUrl
    std::unordered_map<std::string, std::string> reverseMap_;  // longUrl -> shortCode
    uint64_t nextId_;                        // Next ID to use for encoding
    
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
};

#endif // URL_SHORTENER_H

