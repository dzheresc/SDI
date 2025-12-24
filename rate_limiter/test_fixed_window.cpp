#include "fixed_window.h"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <iomanip>

void testBasicUsage() {
    std::cout << "=== Fixed Window: Basic Usage Test ===" << std::endl;
    
    // Create a rate limiter: 5 requests per 1 second window
    FixedWindow limiter(5, 1);
    
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
                      << ", time remaining: " << std::fixed << std::setprecision(2)
                      << limiter.getTimeRemainingInWindow() << "s)" << std::endl;
        } else {
            denied++;
            std::cout << "Request " << i + 1 << ": RATE LIMITED (count: " 
                      << limiter.getCurrentCount() << "/" << limiter.getMaxRequests() 
                      << ", time remaining: " << std::fixed << std::setprecision(2)
                      << limiter.getTimeRemainingInWindow() << "s)" << std::endl;
        }
    }
    
    std::cout << "\nSummary: " << allowed << " allowed, " << denied << " denied" << std::endl;
    std::cout << std::endl;
}

void testWindowReset() {
    std::cout << "=== Fixed Window: Window Reset Test ===" << std::endl;
    
    // Small window: 3 requests per 2 seconds
    FixedWindow limiter(3, 2);
    
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
    
    std::cout << "\nWaiting 2.5 seconds for window to reset..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    
    std::cout << "Current count after wait: " << limiter.getCurrentCount() << std::endl;
    std::cout << "Time remaining: " << std::fixed << std::setprecision(2)
              << limiter.getTimeRemainingInWindow() << "s" << std::endl;
    
    if (limiter.tryAllow()) {
        std::cout << "Successfully allowed request after window reset!" << std::endl;
    }
    std::cout << std::endl;
}

void testBurstCapacity() {
    std::cout << "=== Fixed Window: Burst Capacity Test ===" << std::endl;
    
    // Large window: 100 requests per 10 seconds
    FixedWindow limiter(100, 10);
    
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

void testMultipleWindows() {
    std::cout << "=== Fixed Window: Multiple Windows Test ===" << std::endl;
    
    // 3 requests per 1 second window
    FixedWindow limiter(3, 1);
    
    std::cout << "Window 1: Making 5 requests..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        bool allowed = limiter.tryAllow();
        std::cout << "Request " << i + 1 << ": " 
                  << (allowed ? "ALLOWED" : "DENIED") 
                  << " (count: " << limiter.getCurrentCount() << ")" << std::endl;
    }
    
    std::cout << "\nWaiting for window to reset (1.1 seconds)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    
    std::cout << "\nWindow 2: Making 5 requests..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        bool allowed = limiter.tryAllow();
        std::cout << "Request " << i + 1 << ": " 
                  << (allowed ? "ALLOWED" : "DENIED") 
                  << " (count: " << limiter.getCurrentCount() << ")" << std::endl;
    }
    std::cout << std::endl;
}

void testConcurrentAccess() {
    std::cout << "=== Fixed Window: Thread Safety Test ===" << std::endl;
    
    FixedWindow limiter(20, 2);
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

void testTimeRemaining() {
    std::cout << "=== Fixed Window: Time Remaining Test ===" << std::endl;
    
    FixedWindow limiter(10, 3);  // 3 second window
    
    std::cout << "Initial time remaining: " << std::fixed << std::setprecision(2)
              << limiter.getTimeRemainingInWindow() << "s" << std::endl;
    
    std::cout << "\nMaking a request..." << std::endl;
    limiter.tryAllow();
    std::cout << "Time remaining after request: " << std::fixed << std::setprecision(2)
              << limiter.getTimeRemainingInWindow() << "s" << std::endl;
    
    std::cout << "\nWaiting 1.5 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    std::cout << "Time remaining after wait: " << std::fixed << std::setprecision(2)
              << limiter.getTimeRemainingInWindow() << "s" << std::endl;
    
    std::cout << "\nWaiting for window to reset..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1600));
    std::cout << "Time remaining after reset: " << std::fixed << std::setprecision(2)
              << limiter.getTimeRemainingInWindow() << "s" << std::endl;
    std::cout << std::endl;
}

void testReset() {
    std::cout << "=== Fixed Window: Reset Test ===" << std::endl;
    
    FixedWindow limiter(5, 2);
    
    std::cout << "Adding 3 requests..." << std::endl;
    for (int i = 0; i < 3; ++i) {
        limiter.tryAllow();
    }
    std::cout << "Count before reset: " << limiter.getCurrentCount() << std::endl;
    
    limiter.reset();
    std::cout << "Count after reset: " << limiter.getCurrentCount() << std::endl;
    
    if (limiter.getCurrentCount() == 0) {
        std::cout << "Reset successful - counter is zero!" << std::endl;
    }
    std::cout << std::endl;
}

void runAllFixedWindowTests() {
    try {
        testBasicUsage();
        testWindowReset();
        testBurstCapacity();
        testMultipleWindows();
        testConcurrentAccess();
        testTimeRemaining();
        testReset();
        
        std::cout << "All Fixed Window tests completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error in Fixed Window tests: " << e.what() << std::endl;
        throw;
    }
}

