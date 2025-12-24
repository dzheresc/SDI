#include "leaking_bucket.h"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>

void testBasicUsage() {
    std::cout << "=== Leaking Bucket: Basic Usage Test ===" << std::endl;
    
    // Create a rate limiter: 10 requests capacity, 2 requests per second leak rate
    LeakingBucket limiter(10, 2.0);
    
    std::cout << "Initial queue size: " << limiter.getQueueSize() << std::endl;
    std::cout << "Capacity: " << limiter.getCapacity() << std::endl;
    std::cout << "Leak rate: " << limiter.getLeakRate() << " requests/sec" << std::endl;
    std::cout << std::endl;
    
    // Try to add requests
    int allowed = 0;
    int denied = 0;
    
    for (int i = 0; i < 15; ++i) {
        if (limiter.tryAdd()) {
            allowed++;
            std::cout << "Request " << i + 1 << ": ALLOWED (queue size: " 
                      << limiter.getQueueSize() << ")" << std::endl;
        } else {
            denied++;
            std::cout << "Request " << i + 1 << ": RATE LIMITED (queue size: " 
                      << limiter.getQueueSize() << ")" << std::endl;
        }
    }
    
    std::cout << "\nSummary: " << allowed << " allowed, " << denied << " denied" << std::endl;
    std::cout << std::endl;
}

void testLeakRate() {
    std::cout << "=== Leaking Bucket: Leak Rate Test ===" << std::endl;
    
    // Small bucket: 5 requests capacity, 1 request per second leak rate
    LeakingBucket limiter(5, 1.0);
    
    std::cout << "Adding 5 requests to fill the bucket..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        limiter.tryAdd();
        std::cout << "Added request " << i + 1 << ", queue size: " 
                  << limiter.getQueueSize() << std::endl;
    }
    
    std::cout << "\nWaiting 3 seconds for requests to leak..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    std::cout << "Queue size after 3 seconds: " 
              << limiter.getQueueSize() << std::endl;
    
    // Try to add more requests now that some have leaked
    if (limiter.tryAdd()) {
        std::cout << "Successfully added a request after leak!" << std::endl;
    }
    std::cout << std::endl;
}

void testBurstCapacity() {
    std::cout << "=== Leaking Bucket: Burst Capacity Test ===" << std::endl;
    
    // Large capacity allows bursts
    LeakingBucket limiter(100, 10.0);
    
    std::cout << "Trying to add 50 requests at once..." << std::endl;
    if (limiter.tryAdd(50)) {
        std::cout << "Burst of 50 requests ALLOWED" << std::endl;
        std::cout << "Queue size: " << limiter.getQueueSize() << std::endl;
    } else {
        std::cout << "Burst of 50 requests DENIED" << std::endl;
    }
    
    std::cout << "\nTrying to add 60 more requests..." << std::endl;
    if (limiter.tryAdd(60)) {
        std::cout << "Burst of 60 requests ALLOWED" << std::endl;
    } else {
        std::cout << "Burst of 60 requests DENIED (queue size: " 
                  << limiter.getQueueSize() << ", capacity: " 
                  << limiter.getCapacity() << ")" << std::endl;
    }
    std::cout << std::endl;
}

void testSmoothOutput() {
    std::cout << "=== Leaking Bucket: Smooth Output Rate Test ===" << std::endl;
    
    LeakingBucket limiter(20, 3.0);  // 3 requests per second leak rate
    
    std::cout << "Filling bucket with 20 requests..." << std::endl;
    for (int i = 0; i < 20; ++i) {
        limiter.tryAdd();
    }
    std::cout << "Initial queue size: " << limiter.getQueueSize() << std::endl;
    
    std::cout << "\nMonitoring queue size over 5 seconds..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        int queueSize = limiter.getQueueSize();
        std::cout << "After " << (i + 1) << " second(s): queue size = " 
                  << queueSize << std::endl;
    }
    std::cout << std::endl;
}

void testConcurrentAccess() {
    std::cout << "=== Leaking Bucket: Thread Safety Test ===" << std::endl;
    
    LeakingBucket limiter(30, 5.0);
    std::atomic<int> allowed(0);
    std::atomic<int> denied(0);
    
    auto worker = [&limiter, &allowed, &denied](int id) {
        for (int i = 0; i < 15; ++i) {
            if (limiter.tryAdd()) {
                allowed++;
            } else {
                denied++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    };
    
    std::vector<std::thread> threads;
    const int numThreads = 3;
    
    std::cout << "Starting " << numThreads << " threads, each making 15 requests..." << std::endl;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Total allowed: " << allowed.load() << std::endl;
    std::cout << "Total denied: " << denied.load() << std::endl;
    std::cout << "Final queue size: " << limiter.getQueueSize() << std::endl;
    std::cout << std::endl;
}

void testReset() {
    std::cout << "=== Leaking Bucket: Reset Test ===" << std::endl;
    
    LeakingBucket limiter(10, 2.0);
    
    std::cout << "Adding 5 requests..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        limiter.tryAdd();
    }
    std::cout << "Queue size before reset: " << limiter.getQueueSize() << std::endl;
    
    limiter.reset();
    std::cout << "Queue size after reset: " << limiter.getQueueSize() << std::endl;
    
    if (limiter.getQueueSize() == 0) {
        std::cout << "Reset successful - bucket is empty!" << std::endl;
    }
    std::cout << std::endl;
}

void runAllLeakingBucketTests() {
    try {
        testBasicUsage();
        testLeakRate();
        testBurstCapacity();
        testSmoothOutput();
        testConcurrentAccess();
        testReset();
        
        std::cout << "All Leaking Bucket tests completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error in Leaking Bucket tests: " << e.what() << std::endl;
        throw;
    }
}

