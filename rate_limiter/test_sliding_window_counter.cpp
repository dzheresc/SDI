#include "sliding_window_counter.h"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <iomanip>

void testBasicUsage() {
    std::cout << "=== Sliding Window Counter: Basic Usage Test ===" << std::endl;
    
    // Create a rate limiter: 5 requests per 1 second window, 10 sub-windows
    SlidingWindowCounter limiter(5, 1, 10);
    
    std::cout << "Max requests per window: " << limiter.getMaxRequests() << std::endl;
    std::cout << "Window size: " << limiter.getWindowSizeSeconds() << " seconds" << std::endl;
    std::cout << "Number of sub-windows: " << limiter.getNumSubWindows() << std::endl;
    std::cout << "Current count: " << std::fixed << std::setprecision(2)
              << limiter.getCurrentCount() << std::endl;
    std::cout << std::endl;
    
    // Try to make requests
    int allowed = 0;
    int denied = 0;
    
    for (int i = 0; i < 8; ++i) {
        if (limiter.tryAllow()) {
            allowed++;
            std::cout << "Request " << i + 1 << ": ALLOWED (count: " 
                      << std::fixed << std::setprecision(2)
                      << limiter.getCurrentCount() << "/" << limiter.getMaxRequests() << ")" << std::endl;
        } else {
            denied++;
            std::cout << "Request " << i + 1 << ": RATE LIMITED (count: " 
                      << std::fixed << std::setprecision(2)
                      << limiter.getCurrentCount() << "/" << limiter.getMaxRequests() << ")" << std::endl;
        }
    }
    
    std::cout << "\nSummary: " << allowed << " allowed, " << denied << " denied" << std::endl;
    std::cout << std::endl;
}

void testSlidingWindowBehavior() {
    std::cout << "=== Sliding Window Counter: Sliding Window Behavior Test ===" << std::endl;
    
    // Small window: 3 requests per 2 seconds, 5 sub-windows
    SlidingWindowCounter limiter(3, 2, 5);
    
    std::cout << "Filling window with 3 requests..." << std::endl;
    for (int i = 0; i < 3; ++i) {
        limiter.tryAllow();
        std::cout << "Request " << i + 1 << " allowed, count: " 
                  << std::fixed << std::setprecision(2)
                  << limiter.getCurrentCount() << std::endl;
    }
    
    std::cout << "\nTrying 4th request (should be denied)..." << std::endl;
    if (!limiter.tryAllow()) {
        std::cout << "Correctly denied 4th request" << std::endl;
    }
    
    std::cout << "\nWaiting 1 second (requests should start expiring)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Current count after 1 second: " << std::fixed << std::setprecision(2)
              << limiter.getCurrentCount() << std::endl;
    
    std::cout << "\nWaiting another 1.2 seconds (all requests should expire)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    std::cout << "Current count after expiration: " << std::fixed << std::setprecision(2)
              << limiter.getCurrentCount() << std::endl;
    
    if (limiter.tryAllow()) {
        std::cout << "Successfully allowed request after window expired!" << std::endl;
    }
    std::cout << std::endl;
}

void testBurstCapacity() {
    std::cout << "=== Sliding Window Counter: Burst Capacity Test ===" << std::endl;
    
    // Large window: 100 requests per 10 seconds, 20 sub-windows
    SlidingWindowCounter limiter(100, 10, 20);
    
    std::cout << "Trying to allow 50 requests at once..." << std::endl;
    if (limiter.tryAllow(50)) {
        std::cout << "Burst of 50 requests ALLOWED" << std::endl;
        std::cout << "Current count: " << std::fixed << std::setprecision(2)
                  << limiter.getCurrentCount() << std::endl;
    } else {
        std::cout << "Burst of 50 requests DENIED" << std::endl;
    }
    
    std::cout << "\nTrying to allow 60 more requests..." << std::endl;
    if (limiter.tryAllow(60)) {
        std::cout << "Burst of 60 requests ALLOWED" << std::endl;
    } else {
        std::cout << "Burst of 60 requests DENIED (current count: " 
                  << std::fixed << std::setprecision(2)
                  << limiter.getCurrentCount() << ", max: " 
                  << limiter.getMaxRequests() << ")" << std::endl;
    }
    std::cout << std::endl;
}

void testGradualExpiration() {
    std::cout << "=== Sliding Window Counter: Gradual Expiration Test ===" << std::endl;
    
    // 5 requests per 3 second window, 10 sub-windows
    SlidingWindowCounter limiter(5, 3, 10);
    
    std::cout << "Adding 5 requests at time 0..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        limiter.tryAllow();
    }
    std::cout << "Initial count: " << std::fixed << std::setprecision(2)
              << limiter.getCurrentCount() << std::endl;
    
    std::cout << "\nWaiting 1 second..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Count after 1 second: " << std::fixed << std::setprecision(2)
              << limiter.getCurrentCount() << std::endl;
    
    std::cout << "\nWaiting another 1 second..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Count after 2 seconds: " << std::fixed << std::setprecision(2)
              << limiter.getCurrentCount() << std::endl;
    
    std::cout << "\nWaiting another 1.1 seconds (window should expire)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    std::cout << "Count after expiration: " << std::fixed << std::setprecision(2)
              << limiter.getCurrentCount() << std::endl;
    
    if (limiter.getCurrentCount() < 0.1) {
        std::cout << "All requests expired correctly!" << std::endl;
    }
    std::cout << std::endl;
}

void testContinuousRequests() {
    std::cout << "=== Sliding Window Counter: Continuous Requests Test ===" << std::endl;
    
    // 3 requests per 2 second window, 8 sub-windows
    SlidingWindowCounter limiter(3, 2, 8);
    
    std::cout << "Making requests continuously over 4 seconds..." << std::endl;
    for (int i = 0; i < 10; ++i) {
        bool allowed = limiter.tryAllow();
        std::cout << "Request " << (i + 1) << ": " 
                  << (allowed ? "ALLOWED" : "DENIED") 
                  << " (count: " << std::fixed << std::setprecision(2)
                  << limiter.getCurrentCount() << ")" << std::endl;
        
        // Wait 0.5 seconds between requests
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << std::endl;
}

void testConcurrentAccess() {
    std::cout << "=== Sliding Window Counter: Thread Safety Test ===" << std::endl;
    
    SlidingWindowCounter limiter(20, 2, 10);
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
    std::cout << "Current count: " << std::fixed << std::setprecision(2)
              << limiter.getCurrentCount() << std::endl;
    std::cout << std::endl;
}

void testReset() {
    std::cout << "=== Sliding Window Counter: Reset Test ===" << std::endl;
    
    SlidingWindowCounter limiter(5, 2, 5);
    
    std::cout << "Adding 3 requests..." << std::endl;
    for (int i = 0; i < 3; ++i) {
        limiter.tryAllow();
    }
    std::cout << "Count before reset: " << std::fixed << std::setprecision(2)
              << limiter.getCurrentCount() << std::endl;
    
    limiter.reset();
    std::cout << "Count after reset: " << std::fixed << std::setprecision(2)
              << limiter.getCurrentCount() << std::endl;
    
    if (limiter.getCurrentCount() < 0.01) {
        std::cout << "Reset successful - counter is zero!" << std::endl;
    }
    std::cout << std::endl;
}

void testSubWindowCount() {
    std::cout << "=== Sliding Window Counter: Sub-Window Count Test ===" << std::endl;
    
    std::cout << "Testing with different numbers of sub-windows..." << std::endl;
    
    // Test with 5 sub-windows
    SlidingWindowCounter limiter1(10, 5, 5);
    std::cout << "\n5 sub-windows:" << std::endl;
    std::cout << "Sub-window size: " << std::fixed << std::setprecision(2)
              << (5.0 / 5) << " seconds" << std::endl;
    
    // Test with 10 sub-windows
    SlidingWindowCounter limiter2(10, 5, 10);
    std::cout << "\n10 sub-windows:" << std::endl;
    std::cout << "Sub-window size: " << std::fixed << std::setprecision(2)
              << (5.0 / 10) << " seconds" << std::endl;
    
    // Test with 20 sub-windows
    SlidingWindowCounter limiter3(10, 5, 20);
    std::cout << "\n20 sub-windows:" << std::endl;
    std::cout << "Sub-window size: " << std::fixed << std::setprecision(2)
              << (5.0 / 20) << " seconds" << std::endl;
    
    std::cout << "\nMore sub-windows = better accuracy but more memory usage" << std::endl;
    std::cout << std::endl;
}

void testWeightedCounting() {
    std::cout << "=== Sliding Window Counter: Weighted Counting Test ===" << std::endl;
    
    // 10 requests per 2 second window, 4 sub-windows (0.5s each)
    SlidingWindowCounter limiter(10, 2, 4);
    
    std::cout << "Adding 5 requests..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        limiter.tryAllow();
    }
    std::cout << "Count immediately after: " << std::fixed << std::setprecision(2)
              << limiter.getCurrentCount() << std::endl;
    
    std::cout << "\nWaiting 0.5 seconds (one sub-window expires)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Count after 0.5s: " << std::fixed << std::setprecision(2)
              << limiter.getCurrentCount() << std::endl;
    
    std::cout << "\nWaiting another 0.5 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Count after 1.0s: " << std::fixed << std::setprecision(2)
              << limiter.getCurrentCount() << std::endl;
    
    std::cout << "\nNote: Count decreases gradually due to weighted calculation" << std::endl;
    std::cout << std::endl;
}

void runAllSlidingWindowCounterTests() {
    try {
        testBasicUsage();
        testSlidingWindowBehavior();
        testBurstCapacity();
        testGradualExpiration();
        testContinuousRequests();
        testConcurrentAccess();
        testReset();
        testSubWindowCount();
        testWeightedCounting();
        
        std::cout << "All Sliding Window Counter tests completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error in Sliding Window Counter tests: " << e.what() << std::endl;
        throw;
    }
}

