#include "scache.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <set>
#include <atomic>
#include <mutex>
#include <cassert>
#include <stdexcept>
#include <cstring>

// Test counter for tracking test results
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

void testConstructor() {
    std::cout << "\n=== Constructor Tests ===" << std::endl;
    
    StringsCache cache;
    
    // Constructor should intern empty string by default
    ASSERT(cache.size() == 1, "Constructor interns empty string (size == 1)");
    ASSERT(!cache.empty(), "Cache is not empty after construction (empty string interned)");
    
    // Verify empty string is interned
    CachedString emptyCached(0);
    std::string_view emptyView = cache.resolve(emptyCached);
    ASSERT(emptyView.empty(), "Empty string is interned at index 0");
    ASSERT(emptyView == "", "Empty string resolves correctly");
    
    std::cout << std::endl;
}

void testIntern() {
    std::cout << "\n=== intern() Tests ===" << std::endl;
    
    StringsCache cache;
    
    // Intern first string (index 1, since 0 is empty string)
    CachedString cached1 = cache.intern("hello");
    ASSERT(cached1.index == 1, "First intern returns index 1 (0 is empty string)");
    
    // Intern second string
    CachedString cached2 = cache.intern("world");
    ASSERT(cached2.index == 2, "Second intern returns index 2");
    
    // Intern duplicate
    CachedString cached3 = cache.intern("hello");
    ASSERT(cached3.index == 1, "Duplicate string returns same index");
    ASSERT(cached1.index == cached3.index, "Duplicate strings share same CachedString");
    
    // Verify size
    ASSERT(cache.size() == 3, "Cache has 3 strings (empty + hello + world)");
    
    std::cout << std::endl;
}

void testInternStringView() {
    std::cout << "\n=== intern(std::string_view) Tests ===" << std::endl;
    
    StringsCache cache;
    
    std::string str = "test_string";
    std::string_view sv(str);
    
    CachedString cached = cache.intern(sv);
    ASSERT(cached.index == 1, "intern with string_view works");
    
    // Test with temporary string_view
    CachedString cached2 = cache.intern(std::string_view("temporary"));
    ASSERT(cached2.index == 2, "intern with temporary string_view works");
    
    // Test duplicate
    CachedString cached3 = cache.intern(std::string_view("test_string"));
    ASSERT(cached3.index == 1, "Duplicate string_view returns same index");
    
    std::cout << std::endl;
}

void testInternEmptyString() {
    std::cout << "\n=== intern() Empty String Tests ===" << std::endl;
    
    StringsCache cache;
    
    // Empty string is already interned at construction
    CachedString empty1 = cache.intern("");
    ASSERT(empty1.index == 0, "Empty string returns index 0");
    
    // Intern empty string again
    CachedString empty2 = cache.intern("");
    ASSERT(empty2.index == 0, "Empty string duplicate returns index 0");
    
    ASSERT(cache.size() == 1, "Only one string in cache (empty string)");
    
    std::cout << std::endl;
}

void testResolve() {
    std::cout << "\n=== resolve() Tests ===" << std::endl;
    
    StringsCache cache;
    
    CachedString cached1 = cache.intern("resolve1");
    CachedString cached2 = cache.intern("resolve2");
    CachedString cached3 = cache.intern("resolve3");
    
    std::string_view view1 = cache.resolve(cached1);
    std::string_view view2 = cache.resolve(cached2);
    std::string_view view3 = cache.resolve(cached3);
    
    ASSERT(view1 == "resolve1", "resolve returns correct string_view for cached1");
    ASSERT(view2 == "resolve2", "resolve returns correct string_view for cached2");
    ASSERT(view3 == "resolve3", "resolve returns correct string_view for cached3");
    
    // Test that resolve returns same view for same CachedString
    std::string_view view1_again = cache.resolve(cached1);
    ASSERT(view1.data() == view1_again.data(), "resolve returns same view for same CachedString");
    ASSERT(view1 == view1_again, "resolve returns same content for same CachedString");
    
    // Test empty string resolve
    CachedString emptyCached(0);
    std::string_view emptyView = cache.resolve(emptyCached);
    ASSERT(emptyView.empty(), "resolve(0) returns empty string");
    
    std::cout << std::endl;
}

void testResolveInvalidIndex() {
    std::cout << "\n=== resolve() Invalid Index Tests ===" << std::endl;
    
    StringsCache cache;
    
    cache.intern("test");
    
    // Test out of range index
    try {
        CachedString invalid(9999);
        cache.resolve(invalid);
        ASSERT(false, "resolve(invalid) should throw exception");
    } catch (const std::runtime_error& e) {
        ASSERT(true, "resolve(invalid) correctly throws runtime_error");
    }
    
    // Test index equal to size
    try {
        CachedString invalid(cache.size());
        cache.resolve(invalid);
        ASSERT(false, "resolve(size()) should throw exception");
    } catch (const std::runtime_error& e) {
        ASSERT(true, "resolve(size()) correctly throws runtime_error");
    }
    
    std::cout << std::endl;
}

void testSize() {
    std::cout << "\n=== size() Tests ===" << std::endl;
    
    StringsCache cache;
    
    // Starts with 1 (empty string)
    ASSERT(cache.size() == 1, "New cache has size 1 (empty string)");
    
    cache.intern("str1");
    ASSERT(cache.size() == 2, "Cache has size 2 after one intern");
    
    cache.intern("str2");
    ASSERT(cache.size() == 3, "Cache has size 3 after two unique interns");
    
    cache.intern("str1");  // Duplicate
    ASSERT(cache.size() == 3, "Cache size unchanged after duplicate intern");
    
    cache.intern("str3");
    cache.intern("str4");
    ASSERT(cache.size() == 5, "Cache has size 5 after 4 unique interns");
    
    std::cout << std::endl;
}

void testEmpty() {
    std::cout << "\n=== empty() Tests ===" << std::endl;
    
    StringsCache cache;
    
    // Cache is not empty because empty string is interned
    ASSERT(!cache.empty(), "New cache is not empty (empty string interned)");
    
    // After intern, still not empty
    cache.intern("test");
    ASSERT(!cache.empty(), "Cache is not empty after intern");
    
    // Note: empty() returns true only if size() == 0, but constructor interns empty string
    // So empty() will always return false for a valid cache
    
    std::cout << std::endl;
}

void testDuplicateDetection() {
    std::cout << "\n=== Duplicate Detection Tests ===" << std::endl;
    
    StringsCache cache;
    
    CachedString cached1 = cache.intern("duplicate");
    CachedString cached2 = cache.intern("duplicate");
    CachedString cached3 = cache.intern("duplicate");
    
    ASSERT(cached1.index == cached2.index, "Duplicate strings return same index");
    ASSERT(cached2.index == cached3.index, "All duplicates return same index");
    ASSERT(cache.size() == 2, "Only 2 unique strings (empty + duplicate)");
    
    // Verify they resolve to same memory
    std::string_view view1 = cache.resolve(cached1);
    std::string_view view2 = cache.resolve(cached2);
    ASSERT(view1.data() == view2.data(), "Duplicate strings share same memory");
    
    std::cout << std::endl;
}

void testMemoryBlocks() {
    std::cout << "\n=== Memory Block Management Tests ===" << std::endl;
    
    StringsCache cache;
    
    // Test that strings are stored in blocks
    // BLOCK_SIZE is 64KB, so we need to add strings that exceed this
    
    const size_t largeStringSize = 1000;
    std::string largeString(largeStringSize, 'A');
    
    // Add many large strings to trigger new block allocation
    std::vector<CachedString> cachedStrings;
    for (int i = 0; i < 100; ++i) {
        std::string str = largeString + std::to_string(i);
        cachedStrings.push_back(cache.intern(str));
    }
    
    ASSERT(cache.size() == 101, "Cache has 101 strings (empty + 100 large strings)");
    
    // Verify all can be resolved
    bool allResolved = true;
    for (size_t i = 0; i < cachedStrings.size(); ++i) {
        try {
            std::string_view view = cache.resolve(cachedStrings[i]);
            if (view.empty() && i > 0) {
                allResolved = false;
                break;
            }
        } catch (...) {
            allResolved = false;
            break;
        }
    }
    
    ASSERT(allResolved, "All strings in multiple blocks can be resolved");
    
    std::cout << std::endl;
}

void testCachedStringStruct() {
    std::cout << "\n=== CachedString Struct Tests ===" << std::endl;
    
    // Test constructor
    CachedString cached1(42);
    ASSERT(cached1.index == 42, "CachedString constructor sets index correctly");
    
    CachedString cached2(0);
    ASSERT(cached2.index == 0, "CachedString constructor works with 0");
    
    CachedString cached3(1000);
    ASSERT(cached3.index == 1000, "CachedString constructor works with large index");
    
    std::cout << std::endl;
}

void testStringViewStability() {
    std::cout << "\n=== String View Stability Tests ===" << std::endl;
    
    StringsCache cache;
    
    std::string temp = "temporary_string";
    CachedString cached = cache.intern(temp);
    
    // Clear original string
    temp.clear();
    temp = "different";
    
    // Resolved view should still be valid
    std::string_view view = cache.resolve(cached);
    ASSERT(view == "temporary_string", "Resolved view remains valid after original string changes");
    ASSERT(view != temp, "Resolved view is independent of original string");
    
    std::cout << std::endl;
}

void testConcurrentAccess() {
    std::cout << "\n=== Concurrent Access Tests ===" << std::endl;
    
    StringsCache cache;
    
    std::atomic<int> successCount(0);
    std::atomic<int> failCount(0);
    std::set<size_t> uniqueIndices;
    std::mutex indicesMutex;
    
    auto worker = [&cache, &successCount, &failCount, &uniqueIndices, &indicesMutex](int threadId) {
        for (int i = 0; i < 100; ++i) {
            try {
                std::string key = "thread_" + std::to_string(threadId) + "_key_" + std::to_string(i);
                CachedString cached = cache.intern(key);
                
                std::string_view view = cache.resolve(cached);
                if (view == key) {
                    {
                        std::lock_guard<std::mutex> lock(indicesMutex);
                        uniqueIndices.insert(cached.index);
                    }
                    successCount++;
                } else {
                    failCount++;
                }
            } catch (...) {
                failCount++;
            }
        }
    };
    
    std::vector<std::thread> threads;
    const int numThreads = 10;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Concurrent operations - Success: " << successCount.load() 
              << ", Failed: " << failCount.load() << std::endl;
    std::cout << "Unique indices: " << uniqueIndices.size() << std::endl;
    std::cout << "Cache size: " << cache.size() << std::endl;
    
    ASSERT(failCount.load() == 0, "All concurrent operations succeeded");
    ASSERT(successCount.load() == numThreads * 100, "All operations completed");
    
    std::cout << std::endl;
}

void testLargeScale() {
    std::cout << "\n=== Large Scale Tests ===" << std::endl;
    
    StringsCache cache;
    
    const int numUnique = 10000;
    const int numTotal = 100000;
    
    std::cout << "Interning " << numTotal << " strings (" << numUnique << " unique)..." << std::endl;
    
    std::vector<CachedString> cachedStrings;
    for (int i = 0; i < numTotal; ++i) {
        int index = i % numUnique;
        std::string str = "large_scale_" + std::to_string(index);
        cachedStrings.push_back(cache.intern(str));
    }
    
    // Should have 1 (empty) + numUnique unique strings
    ASSERT(cache.size() == numUnique + 1, "Large scale: correct number of unique strings");
    
    // Verify all can be resolved
    bool allResolved = true;
    for (const auto& cached : cachedStrings) {
        try {
            std::string_view view = cache.resolve(cached);
            if (view.empty() && cached.index != 0) {
                allResolved = false;
                break;
            }
        } catch (...) {
            allResolved = false;
            break;
        }
    }
    
    ASSERT(allResolved, "Large scale: all strings can be resolved");
    
    // Verify duplicates share same index
    bool duplicatesShareIndex = true;
    for (int i = 0; i < numUnique; ++i) {
        std::string str = "large_scale_" + std::to_string(i);
        CachedString cached1 = cache.intern(str);
        CachedString cached2 = cache.intern(str);
        if (cached1.index != cached2.index) {
            duplicatesShareIndex = false;
            break;
        }
    }
    
    ASSERT(duplicatesShareIndex, "Large scale: duplicates share same index");
    
    std::cout << std::endl;
}

void testSpecialCharacters() {
    std::cout << "\n=== Special Characters Tests ===" << std::endl;
    
    StringsCache cache;
    
    // Test with various special characters
    std::vector<std::string> specialStrings = {
        "string with spaces",
        "string\twith\ttabs",
        "string\nwith\nnewlines",
        "string\rwith\rcarriage",
        "string with \"quotes\"",
        "string with 'apostrophes'",
        "string with unicode: 你好世界",
        "string with symbols: !@#$%^&*()",
        "string with null: \0embedded",
    };
    
    std::vector<CachedString> cachedStrings;
    for (const auto& str : specialStrings) {
        cachedStrings.push_back(cache.intern(str));
    }
    
    // Verify all can be resolved correctly
    bool allCorrect = true;
    for (size_t i = 0; i < specialStrings.size(); ++i) {
        std::string_view view = cache.resolve(cachedStrings[i]);
        if (view != specialStrings[i]) {
            allCorrect = false;
            break;
        }
    }
    
    ASSERT(allCorrect, "Special characters are handled correctly");
    ASSERT(cache.size() == specialStrings.size() + 1, "All special strings are stored");
    
    std::cout << std::endl;
}

void testVeryLongStrings() {
    std::cout << "\n=== Very Long Strings Tests ===" << std::endl;
    
    StringsCache cache;
    
    // Create strings of various lengths
    std::string shortStr = "short";
    std::string mediumStr(1000, 'M');
    std::string longStr(10000, 'L');
    std::string veryLongStr(50000, 'V');
    
    CachedString cached1 = cache.intern(shortStr);
    CachedString cached2 = cache.intern(mediumStr);
    CachedString cached3 = cache.intern(longStr);
    CachedString cached4 = cache.intern(veryLongStr);
    
    ASSERT(cache.resolve(cached1) == shortStr, "Short string interned correctly");
    ASSERT(cache.resolve(cached2) == mediumStr, "Medium string interned correctly");
    ASSERT(cache.resolve(cached3) == longStr, "Long string interned correctly");
    ASSERT(cache.resolve(cached4) == veryLongStr, "Very long string interned correctly");
    
    // Test duplicate
    CachedString cached5 = cache.intern(veryLongStr);
    ASSERT(cached4.index == cached5.index, "Very long duplicate returns same index");
    
    std::cout << std::endl;
}

void runAllComprehensiveTests() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Comprehensive StringsCache Test Suite" << std::endl;
    std::cout << "  Based on scache.h API" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testsPassed = 0;
    testsFailed = 0;
    
    try {
        testConstructor();
        testIntern();
        testInternStringView();
        testInternEmptyString();
        testResolve();
        testResolveInvalidIndex();
        testSize();
        testEmpty();
        testDuplicateDetection();
        testMemoryBlocks();
        testCachedStringStruct();
        testStringViewStability();
        testConcurrentAccess();
        testLargeScale();
        testSpecialCharacters();
        testVeryLongStrings();
        
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

