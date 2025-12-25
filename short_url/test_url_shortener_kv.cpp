#include "url_shortener_kv.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
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

void testBasicShortenKV() {
    std::cout << "\n=== KeyValue Store: Basic Shorten Test ===" << std::endl;
    
    UrlShortenerKV shortener;
    
    std::string longUrl = "https://www.example.com/very/long/url/path";
    std::string shortUrl = shortener.shorten(longUrl);
    
    std::cout << "Long URL: " << longUrl << std::endl;
    std::cout << "Short URL: " << shortUrl << std::endl;
    
    ASSERT(!shortUrl.empty(), "Short URL is not empty");
    ASSERT(shortUrl.find("https://short.ly/") == 0, "Short URL starts with base URL");
    ASSERT(shortener.size() == 1, "Database has 1 URL");
    
    std::cout << std::endl;
}

void testExpandKV() {
    std::cout << "\n=== KeyValue Store: Expand Test ===" << std::endl;
    
    UrlShortenerKV shortener;
    
    std::string longUrl = "https://www.google.com/search?q=test";
    std::string shortUrl = shortener.shorten(longUrl);
    
    // Extract short code
    std::string shortCode = shortUrl.substr(18);  // Remove base URL
    
    // Test expand with short code
    std::string expanded = shortener.expand(shortCode);
    ASSERT(expanded == longUrl, "Expand returns original URL");
    
    // Test expandUrl with full short URL
    std::string expanded2 = shortener.expandUrl(shortUrl);
    ASSERT(expanded2 == longUrl, "expandUrl returns original URL");
    
    std::cout << std::endl;
}

void testDuplicateUrlsKV() {
    std::cout << "\n=== KeyValue Store: Duplicate URLs Test ===" << std::endl;
    
    UrlShortenerKV shortener;
    
    std::string longUrl = "https://www.example.com";
    
    std::string shortUrl1 = shortener.shorten(longUrl);
    std::string shortUrl2 = shortener.shorten(longUrl);  // Duplicate
    
    ASSERT(shortUrl1 == shortUrl2, "Duplicate URLs return same short URL");
    ASSERT(shortener.size() == 1, "Only one entry for duplicate URLs");
    
    std::cout << std::endl;
}

void testMultipleUrlsKV() {
    std::cout << "\n=== KeyValue Store: Multiple URLs Test ===" << std::endl;
    
    UrlShortenerKV shortener;
    
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
    
    std::cout << std::endl;
}

void testSaveAndLoadKV() {
    std::cout << "\n=== KeyValue Store: Save and Load Test ===" << std::endl;
    
    const std::string filename = "test_urls_kv.csv";
    
    // Create shortener and add URLs
    {
        UrlShortenerKV shortener;
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
        UrlShortenerKV shortener2;
        bool loaded = shortener2.loadFromFile(filename);
        ASSERT(loaded, "Load from file successful");
        ASSERT(shortener2.size() == 3, "Loaded shortener has 3 URLs");
        
        // Verify URLs can be expanded
        std::string expanded1 = shortener2.expand("1");
        std::string expanded2 = shortener2.expand("2");
        ASSERT(!expanded1.empty(), "First URL can be expanded");
        ASSERT(!expanded2.empty(), "Second URL can be expanded");
    }
    
    // Clean up
    std::remove(filename.c_str());
    
    std::cout << std::endl;
}

void testServerManagement() {
    std::cout << "\n=== KeyValue Store: Server Management Test ===" << std::endl;
    
    UrlShortenerKV shortener;
    
    // Add servers
    ASSERT(shortener.addServer("server2"), "Add server2 successful");
    ASSERT(shortener.addServer("server3"), "Add server3 successful");
    
    auto servers = shortener.getServers();
    ASSERT(servers.size() >= 2, "At least 2 servers in cluster");
    
    // Shorten a URL and check which server it's on
    std::string shortUrl = shortener.shorten("https://www.example.com");
    std::string shortCode = shortUrl.substr(18);
    std::string server = shortener.getServerForKey(shortCode);
    
    ASSERT(!server.empty(), "Server assignment works");
    std::cout << "Short code '" << shortCode << "' assigned to: " << server << std::endl;
    
    // Remove a server
    ASSERT(shortener.removeServer("server2"), "Remove server2 successful");
    
    std::cout << std::endl;
}

void testDistributedStorage() {
    std::cout << "\n=== KeyValue Store: Distributed Storage Test ===" << std::endl;
    
    UrlShortenerKV shortener;
    
    // Add multiple servers
    shortener.addServer("server1");
    shortener.addServer("server2");
    shortener.addServer("server3");
    shortener.addServer("server4");
    
    // Shorten multiple URLs - they should be distributed across servers
    std::map<std::string, int> serverCounts;
    for (int i = 0; i < 100; ++i) {
        std::string url = "https://www.example.com/page/" + std::to_string(i);
        std::string shortUrl = shortener.shorten(url);
        std::string shortCode = shortUrl.substr(18);
        std::string server = shortener.getServerForKey(shortCode);
        serverCounts[server]++;
    }
    
    std::cout << "Distribution across servers:" << std::endl;
    for (const auto& pair : serverCounts) {
        std::cout << "  " << pair.first << ": " << pair.second << " URLs" << std::endl;
    }
    
    ASSERT(shortener.size() == 100, "All 100 URLs stored");
    ASSERT(serverCounts.size() >= 2, "URLs distributed across multiple servers");
    
    std::cout << std::endl;
}

void testEmptyAndClearKV() {
    std::cout << "\n=== KeyValue Store: Empty and Clear Test ===" << std::endl;
    
    UrlShortenerKV shortener;
    
    ASSERT(shortener.empty(), "New shortener is empty");
    
    shortener.shorten("https://www.example.com");
    ASSERT(!shortener.empty(), "Shortener is not empty after adding URL");
    ASSERT(shortener.size() == 1, "Size is 1");
    
    shortener.clear();
    ASSERT(shortener.empty(), "Shortener is empty after clear");
    ASSERT(shortener.size() == 0, "Size is 0 after clear");
    
    std::cout << std::endl;
}

void runAllKVTests() {
    std::cout << "========================================" << std::endl;
    std::cout << "  URL Shortener (KeyValue Store) Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testsPassed = 0;
    testsFailed = 0;
    
    try {
        testBasicShortenKV();
        testExpandKV();
        testDuplicateUrlsKV();
        testMultipleUrlsKV();
        testSaveAndLoadKV();
        testServerManagement();
        testDistributedStorage();
        testEmptyAndClearKV();
        
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

