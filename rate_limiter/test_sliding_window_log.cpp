#include "sliding_window_log.h"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <iomanip>

void testBasicUsage() {
    std::cout << "=== Sliding Window Log: Basic Usage Test ===" << std::endl;
    
    // Create a rate limiter: 5 requests per 1 second window
    SlidingWindowLog limiter(5, 1);
    
    std::cout << "Max requests per window: " << limiter.getMaxRequests() << std::endl;
    std::cout << "Window size: " << limiter.getWindowSizeSeconds() << " seconds" << std::endl;
    std::cout << "Current count: " << limiter.getCurrentCount() << std::endl;
    std::cout << std::endl;
    
    // Try to make requests
    int allowed = 0;
    int denied = 0;
    
    for (int i = 0; i < 8; ++i) {
        if (limiter.tryAllow()) {
            allowed++;
            std::cout << "Request " << i + 1 << ": ALLOWED (count: " 
                      << limiter.getCurrentCount() << "/" << limiter.getMaxRequests() 
                      << ", oldest expires in: " << std::fixed << std::setprecision(2)
                      << limiter.getTimeUntilOldestExpires() << "s)" << std::endl;
        } else {
            denied++;
            std::cout << "Request " << i + 1 << ": RATE LIMITED (count: " 
                      << limiter.getCurrentCount() << "/" << limiter.getMaxRequests() 
                      << ", oldest expires in: " << std::fixed << std::setprecision(2)
                      << limiter.getTimeUntilOldestExpires() << "s)" << std::endl;
        }
    }
    
    std::cout << "\nSummary: " << allowed << " allowed, " << denied << " denied" << std::endl;
    std::cout << std::endl;
}

void testSlidingWindowBehavior() {
    std::cout << "=== Sliding Window Log: Sliding Window Behavior Test ===" << std::endl;
    
    // Small window: 3 requests per 2 seconds
    SlidingWindowLog limiter(3, 2);
    
    std::cout << "Filling window with 3 requests..." << std::endl;
    for (int i = 0; i < 3; ++i) {
        limiter.tryAllow();
        std::cout << "Request " << i + 1 << " allowed, count: " 
                  << limiter.getCurrentCount() << std::endl;
    }
    
    std::cout << "\nTrying 4th request (should be denied)..." << std::endl;
    if (!limiter.tryAllow()) {
        std::cout << "Correctly denied 4th request" << std::endl;
    }
    
    std::cout << "\nWaiting 1 second (requests should start expiring)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Current count after 1 second: " << limiter.getCurrentCount() << std::endl;
    std::cout << "Time until oldest expires: " << std::fixed << std::setprecision(2)
              << limiter.getTimeUntilOldestExpires() << "s" << std::endl;
    
    std::cout << "\nWaiting another 1.2 seconds (all requests should expire)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    std::cout << "Current count after expiration: " << limiter.getCurrentCount() << std::endl;
    
    if (limiter.tryAllow()) {
        std::cout << "Successfully allowed request after window expired!" << std::endl;
    }
    std::cout << std::endl;
}

void testBurstCapacity() {
    std::cout << "=== Sliding Window Log: Burst Capacity Test ===" << std::endl;
    
    // Large window: 100 requests per 10 seconds
    SlidingWindowLog limiter(100, 10);
    
    std::cout << "Trying to allow 50 requests at once..." << std::endl;
    if (limiter.tryAllow(50)) {
        std::cout << "Burst of 50 requests ALLOWED" << std::endl;
        std::cout << "Current count: " << limiter.getCurrentCount() << std::endl;
    } else {
        std::cout << "Burst of 50 requests DENIED" << std::endl;
    }
    
    std::cout << "\nTrying to allow 60 more requests..." << std::endl;
    if (limiter.tryAllow(60)) {
        std::cout << "Burst of 60 requests ALLOWED" << std::endl;
    } else {
        std::cout << "Burst of 60 requests DENIED (current count: " 
                  << limiter.getCurrentCount() << ", max: " 
                  << limiter.getMaxRequests() << ")" << std::endl;
    }
    std::cout << std::endl;
}

void testGradualExpiration() {
    std::cout << "=== Sliding Window Log: Gradual Expiration Test ===" << std::endl;
    
    // 5 requests per 3 second window
    SlidingWindowLog limiter(5, 3);
    
    std::cout << "Adding 5 requests at time 0..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        limiter.tryAllow();
    }
    std::cout << "Initial count: " << limiter.getCurrentCount() << std::endl;
    
    std::cout << "\nWaiting 1 second..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Count after 1 second: " << limiter.getCurrentCount() << std::endl;
    std::cout << "Time until oldest expires: " << std::fixed << std::setprecision(2)
              << limiter.getTimeUntilOldestExpires() << "s" << std::endl;
    
    std::cout << "\nWaiting another 1 second..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Count after 2 seconds: " << limiter.getCurrentCount() << std::endl;
    std::cout << "Time until oldest expires: " << std::fixed << std::setprecision(2)
              << limiter.getTimeUntilOldestExpires() << "s" << std::endl;
    
    std::cout << "\nWaiting another 1.1 seconds (window should expire)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    std::cout << "Count after expiration: " << limiter.getCurrentCount() << std::endl;
    
    if (limiter.getCurrentCount() == 0) {
        std::cout << "All requests expired correctly!" << std::endl;
    }
    std::cout << std::endl;
}

void testContinuousRequests() {
    std::cout << "=== Sliding Window Log: Continuous Requests Test ===" << std::endl;
    
    // 3 requests per 2 second window
    SlidingWindowLog limiter(3, 2);
    
    std::cout << "Making requests continuously over 4 seconds..." << std::endl;
    for (int i = 0; i < 10; ++i) {
        bool allowed = limiter.tryAllow();
        std::cout << "Request " << (i + 1) << ": " 
                  << (allowed ? "ALLOWED" : "DENIED") 
                  << " (count: " << limiter.getCurrentCount() << ")" << std::endl;
        
        // Wait 0.5 seconds between requests
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << std::endl;
}

void testConcurrentAccess() {
    std::cout << "=== Sliding Window Log: Thread Safety Test ===" << std::endl;
    
    SlidingWindowLog limiter(20, 2);
    std::atomic<int> allowed(0);
    std::atomic<int> denied(0);
    
    auto worker = [&limiter, &allowed, &denied](int id) {
        for (int i = 0; i < 10; ++i) {
            if (limiter.tryAllow()) {
                allowed++;
            } else {
                denied++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    };
    
    std::vector<std::thread> threads;
    const int numThreads = 4;
    
    std::cout << "Starting " << numThreads << " threads, each making 10 requests..." << std::endl;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Total allowed: " << allowed.load() << std::endl;
    std::cout << "Total denied: " << denied.load() << std::endl;
    std::cout << "Current count: " << limiter.getCurrentCount() << std::endl;
    std::cout << std::endl;
}

void testTimeUntilExpiration() {
    std::cout << "=== Sliding Window Log: Time Until Expiration Test ===" << std::endl;
    
    SlidingWindowLog limiter(10, 3);  // 3 second window
    
    std::cout << "Time until oldest expires (empty): " << std::fixed << std::setprecision(2)
              << limiter.getTimeUntilOldestExpires() << "s" << std::endl;
    
    std::cout << "\nAdding a request..." << std::endl;
    limiter.tryAllow();
    std::cout << "Time until oldest expires: " << std::fixed << std::setprecision(2)
              << limiter.getTimeUntilOldestExpires() << "s" << std::endl;
    
    std::cout << "\nWaiting 1.5 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    std::cout << "Time until oldest expires: " << std::fixed << std::setprecision(2)
              << limiter.getTimeUntilOldestExpires() << "s" << std::endl;
    
    std::cout << "\nWaiting for request to expire..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1600));
    std::cout << "Time until oldest expires (after expiration): " << std::fixed << std::setprecision(2)
              << limiter.getTimeUntilOldestExpires() << "s" << std::endl;
    std::cout << std::endl;
}

void testReset() {
    std::cout << "=== Sliding Window Log: Reset Test ===" << std::endl;
    
    SlidingWindowLog limiter(5, 2);
    
    std::cout << "Adding 3 requests..." << std::endl;
    for (int i = 0; i < 3; ++i) {
        limiter.tryAllow();
    }
    std::cout << "Count before reset: " << limiter.getCurrentCount() << std::endl;
    
    limiter.reset();
    std::cout << "Count after reset: " << limiter.getCurrentCount() << std::endl;
    
    if (limiter.getCurrentCount() == 0) {
        std::cout << "Reset successful - log is empty!" << std::endl;
    }
    std::cout << std::endl;
}

void testAccuracyVsFixedWindow() {
    std::cout << "=== Sliding Window Log: Accuracy vs Fixed Window Test ===" << std::endl;
    
    SlidingWindowLog limiter(3, 2);  // 3 requests per 2 seconds
    
    std::cout << "Adding 3 requests at start..." << std::endl;
    for (int i = 0; i < 3; ++i) {
        limiter.tryAllow();
    }
    std::cout << "Count: " << limiter.getCurrentCount() << std::endl;
    
    std::cout << "\nWaiting 1.9 seconds (almost at window boundary)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1900));
    std::cout << "Count: " << limiter.getCurrentCount() << std::endl;
    std::cout << "Time until oldest expires: " << std::fixed << std::setprecision(2)
              << limiter.getTimeUntilOldestExpires() << "s" << std::endl;
    
    std::cout << "\nTrying to add request (should still be denied)..." << std::endl;
    if (!limiter.tryAllow()) {
        std::cout << "Correctly denied - sliding window is still active" << std::endl;
    }
    
    std::cout << "\nWaiting 0.2 seconds (window should expire)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "Count: " << limiter.getCurrentCount() << std::endl;
    
    if (limiter.tryAllow()) {
        std::cout << "Successfully allowed after window expired!" << std::endl;
    }
    std::cout << std::endl;
}

void runAllSlidingWindowLogTests() {
    try {
        testBasicUsage();
        testSlidingWindowBehavior();
        testBurstCapacity();
        testGradualExpiration();
        testContinuousRequests();
        testConcurrentAccess();
        testTimeUntilExpiration();
        testReset();
        testAccuracyVsFixedWindow();
        
        std::cout << "All Sliding Window Log tests completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error in Sliding Window Log tests: " << e.what() << std::endl;
        throw;
    }
}

