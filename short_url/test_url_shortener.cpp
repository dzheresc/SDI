#include "url_shortener.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <set>
#include <atomic>
#include <cassert>
#include <cstdio>

// Test counter
static int testsPassed = 0;
static int testsFailed = 0;

#define ASSERT(condition, message) \
    do { \
        if (condition) { \
            std::cout << "✓ " << message << std::endl; \
            testsPassed++; \
        } else { \
            std::cout << "✗ " << message << std::endl; \
            testsFailed++; \
        } \
    } while(0)

void testBasicShorten() {
    std::cout << "\n=== Basic Shorten Test ===" << std::endl;
    
    UrlShortener shortener;
    
    std::string longUrl = "https://www.example.com/very/long/url/path";
    std::string shortUrl = shortener.shorten(longUrl);
    
    std::cout << "Long URL: " << longUrl << std::endl;
    std::cout << "Short URL: " << shortUrl << std::endl;
    
    ASSERT(!shortUrl.empty(), "Short URL is not empty");
    ASSERT(shortUrl.find("https://short.ly/") == 0, "Short URL starts with base URL");
    ASSERT(shortener.size() == 1, "Database has 1 URL");
    
    std::cout << std::endl;
}

void testExpand() {
    std::cout << "\n=== Expand Test ===" << std::endl;
    
    UrlShortener shortener;
    
    std::string longUrl = "https://www.google.com/search?q=test";
    std::string shortUrl = shortener.shorten(longUrl);
    
    // Extract short code (remove base URL "https://short.ly/")
    std::string shortCode = shortUrl.substr(18);  // Remove base URL
    
    // Test expand with short code
    std::string expanded = shortener.expand(shortCode);
    ASSERT(expanded == longUrl, "Expand returns original URL");
    
    // Test expandUrl with full short URL
    std::string expanded2 = shortener.expandUrl(shortUrl);
    ASSERT(expanded2 == longUrl, "expandUrl returns original URL");
    
    std::cout << std::endl;
}

void testDuplicateUrls() {
    std::cout << "\n=== Duplicate URLs Test ===" << std::endl;
    
    UrlShortener shortener;
    
    std::string longUrl = "https://www.example.com";
    
    std::string shortUrl1 = shortener.shorten(longUrl);
    std::string shortUrl2 = shortener.shorten(longUrl);  // Duplicate
    
    ASSERT(shortUrl1 == shortUrl2, "Duplicate URLs return same short URL");
    ASSERT(shortener.size() == 1, "Only one entry for duplicate URLs");
    
    std::cout << std::endl;
}

void testBase62Encoding() {
    std::cout << "\n=== Base62 Encoding Test ===" << std::endl;
    
    UrlShortener shortener;
    
    // Test encoding/decoding
    std::vector<uint64_t> testNumbers = {0, 1, 10, 62, 100, 1000, 10000, 1000000};
    
    for (uint64_t num : testNumbers) {
        std::string encoded = UrlShortener::encodeBase62(num);
        uint64_t decoded = UrlShortener::decodeBase62(encoded);
        
        ASSERT(decoded == num, "Base62 encode/decode roundtrip for " + std::to_string(num));
        std::cout << "  " << num << " -> " << encoded << " -> " << decoded << std::endl;
    }
    
    std::cout << std::endl;
}

void testMultipleUrls() {
    std::cout << "\n=== Multiple URLs Test ===" << std::endl;
    
    UrlShortener shortener;
    
    std::vector<std::string> urls = {
        "https://www.example.com/page1",
        "https://www.example.com/page2",
        "https://www.google.com",
        "https://www.github.com/user/repo"
    };
    
    std::vector<std::string> shortUrls;
    for (const auto& url : urls) {
        shortUrls.push_back(shortener.shorten(url));
    }
    
    ASSERT(shortener.size() == urls.size(), "All URLs are stored");
    
    // Verify all can be expanded
    bool allExpanded = true;
    for (size_t i = 0; i < urls.size(); ++i) {
        std::string expanded = shortener.expandUrl(shortUrls[i]);
        if (expanded != urls[i]) {
            allExpanded = false;
            break;
        }
    }
    
    ASSERT(allExpanded, "All URLs can be expanded correctly");
    
    // Verify all short URLs are unique
    std::set<std::string> uniqueShortUrls(shortUrls.begin(), shortUrls.end());
    ASSERT(uniqueShortUrls.size() == urls.size(), "All short URLs are unique");
    
    std::cout << std::endl;
}

void testExists() {
    std::cout << "\n=== Exists Test ===" << std::endl;
    
    UrlShortener shortener;
    
    std::string longUrl = "https://www.test.com";
    std::string shortUrl = shortener.shorten(longUrl);
    // Extract short code (remove base URL "https://short.ly/")
    std::string shortCode = shortUrl.substr(18);
    
    ASSERT(shortener.exists(shortCode), "exists returns true for existing code");
    ASSERT(!shortener.exists("nonexistent"), "exists returns false for non-existent code");
    ASSERT(!shortener.exists(""), "exists returns false for empty code");
    
    std::cout << std::endl;
}

void testSaveAndLoad() {
    std::cout << "\n=== Save and Load Test ===" << std::endl;
    
    const std::string filename = "test_urls.csv";
    
    // Create shortener and add URLs
    {
        UrlShortener shortener;
        shortener.shorten("https://www.example.com/page1");
        shortener.shorten("https://www.example.com/page2");
        shortener.shorten("https://www.google.com");
        
        ASSERT(shortener.size() == 3, "Original shortener has 3 URLs");
        
        // Save to file
        bool saved = shortener.saveToFile(filename);
        ASSERT(saved, "Save to file successful");
    }
    
    // Load from file
    {
        UrlShortener shortener2;
        bool loaded = shortener2.loadFromFile(filename);
        ASSERT(loaded, "Load from file successful");
        ASSERT(shortener2.size() == 3, "Loaded shortener has 3 URLs");
        
        // Verify URLs can be expanded
        ASSERT(shortener2.expandUrl("https://short.ly/1") != "", "First URL can be expanded");
        ASSERT(shortener2.expandUrl("https://short.ly/2") != "", "Second URL can be expanded");
    }
    
    // Clean up
    std::remove(filename.c_str());
    
    std::cout << std::endl;
}

void testSaveLoadRoundtrip() {
    std::cout << "\n=== Save/Load Roundtrip Test ===" << std::endl;
    
    const std::string filename = "roundtrip_test.csv";
    
    UrlShortener shortener1;
    
    std::vector<std::string> originalUrls = {
        "https://www.example.com/1",
        "https://www.example.com/2",
        "https://www.example.com/3"
    };
    
    std::vector<std::string> shortUrls;
    for (const auto& url : originalUrls) {
        shortUrls.push_back(shortener1.shorten(url));
    }
    
    // Save
    ASSERT(shortener1.saveToFile(filename), "Save successful");
    
    // Load
    UrlShortener shortener2;
    ASSERT(shortener2.loadFromFile(filename), "Load successful");
    
    // Verify all URLs match
    bool allMatch = true;
    for (size_t i = 0; i < originalUrls.size(); ++i) {
        std::string expanded = shortener2.expandUrl(shortUrls[i]);
        if (expanded != originalUrls[i]) {
            allMatch = false;
            break;
        }
    }
    
    ASSERT(allMatch, "All URLs match after save/load roundtrip");
    ASSERT(shortener2.size() == originalUrls.size(), "Size matches after load");
    
    // Clean up
    std::remove(filename.c_str());
    
    std::cout << std::endl;
}

void testEmptyAndClear() {
    std::cout << "\n=== Empty and Clear Test ===" << std::endl;
    
    UrlShortener shortener;
    
    ASSERT(shortener.empty(), "New shortener is empty");
    
    shortener.shorten("https://www.example.com");
    ASSERT(!shortener.empty(), "Shortener is not empty after adding URL");
    ASSERT(shortener.size() == 1, "Size is 1");
    
    shortener.clear();
    ASSERT(shortener.empty(), "Shortener is empty after clear");
    ASSERT(shortener.size() == 0, "Size is 0 after clear");
    
    std::cout << std::endl;
}

void testCustomBaseUrl() {
    std::cout << "\n=== Custom Base URL Test ===" << std::endl;
    
    UrlShortener shortener("https://my.short/");
    
    std::string longUrl = "https://www.example.com";
    std::string shortUrl = shortener.shorten(longUrl);
    
    ASSERT(shortUrl.find("https://my.short/") == 0, "Short URL uses custom base URL");
    
    std::cout << std::endl;
}

void testInvalidInputs() {
    std::cout << "\n=== Invalid Inputs Test ===" << std::endl;
    
    UrlShortener shortener;
    
    // Test empty URL
    try {
        shortener.shorten("");
        ASSERT(false, "shorten with empty URL should throw");
    } catch (const std::invalid_argument& e) {
        ASSERT(true, "shorten with empty URL correctly throws");
    }
    
    // Test expand with non-existent code
    std::string expanded = shortener.expand("nonexistent");
    ASSERT(expanded.empty(), "expand with non-existent code returns empty");
    
    // Test expandUrl with invalid URL
    std::string expanded2 = shortener.expandUrl("https://different.com/abc");
    ASSERT(expanded2.empty(), "expandUrl with invalid URL returns empty");
    
    std::cout << std::endl;
}

void testStats() {
    std::cout << "\n=== Statistics Test ===" << std::endl;
    
    UrlShortener shortener;
    
    shortener.shorten("https://www.example.com/1");
    shortener.shorten("https://www.example.com/2");
    shortener.shorten("https://www.example.com/3");
    
    size_t totalUrls, totalShortCodes;
    shortener.getStats(totalUrls, totalShortCodes);
    
    ASSERT(totalUrls == 3, "Total URLs is 3");
    ASSERT(totalShortCodes == 3, "Total short codes is 3");
    
    std::cout << "Total URLs: " << totalUrls << std::endl;
    std::cout << "Total Short Codes: " << totalShortCodes << std::endl;
    
    std::cout << std::endl;
}

void testLargeScale() {
    std::cout << "\n=== Large Scale Test ===" << std::endl;
    
    UrlShortener shortener;
    
    const int numUrls = 10000;
    std::vector<std::string> shortUrls;
    
    for (int i = 0; i < numUrls; ++i) {
        std::string url = "https://www.example.com/page/" + std::to_string(i);
        shortUrls.push_back(shortener.shorten(url));
    }
    
    ASSERT(shortener.size() == numUrls, "All URLs stored");
    
    // Verify all can be expanded
    bool allExpanded = true;
    for (size_t i = 0; i < shortUrls.size(); ++i) {
        std::string expanded = shortener.expandUrl(shortUrls[i]);
        if (expanded.empty()) {
            allExpanded = false;
            break;
        }
    }
    
    ASSERT(allExpanded, "All URLs can be expanded");
    
    std::cout << "Generated " << numUrls << " short URLs" << std::endl;
    std::cout << std::endl;
}

void runAllTests() {
    std::cout << "========================================" << std::endl;
    std::cout << "  URL Shortener Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testsPassed = 0;
    testsFailed = 0;
    
    try {
        testBasicShorten();
        testExpand();
        testDuplicateUrls();
        testBase62Encoding();
        testMultipleUrls();
        testExists();
        testSaveAndLoad();
        testSaveLoadRoundtrip();
        testEmptyAndClear();
        testCustomBaseUrl();
        testInvalidInputs();
        testStats();
        testLargeScale();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "Test Results:" << std::endl;
        std::cout << "  Passed: " << testsPassed << std::endl;
        std::cout << "  Failed: " << testsFailed << std::endl;
        std::cout << "  Total:  " << (testsPassed + testsFailed) << std::endl;
        std::cout << "========================================" << std::endl;
        
        if (testsFailed == 0) {
            std::cout << "All tests passed!" << std::endl;
        } else {
            std::cout << "Some tests failed!" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        throw;
    }
}

