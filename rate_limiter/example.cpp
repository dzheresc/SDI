#include "token_bucket.h"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

void demonstrateBasicUsage() {
    std::cout << "=== Basic Usage Example ===" << std::endl;
    
    // Create a rate limiter: 10 tokens capacity, 2 tokens per second refill rate
    TokenBucket limiter(10.0, 2.0);
    
    std::cout << "Initial tokens: " << limiter.getAvailableTokens() << std::endl;
    std::cout << "Capacity: " << limiter.getCapacity() << std::endl;
    std::cout << "Refill rate: " << limiter.getRefillRate() << " tokens/sec" << std::endl;
    std::cout << std::endl;
    
    // Try to consume tokens
    int allowed = 0;
    int denied = 0;
    
    for (int i = 0; i < 15; ++i) {
        if (limiter.tryConsume()) {
            allowed++;
            std::cout << "Request " << i + 1 << ": ALLOWED (tokens: " 
                      << limiter.getAvailableTokens() << ")" << std::endl;
        } else {
            denied++;
            std::cout << "Request " << i + 1 << ": RATE LIMITED (tokens: " 
                      << limiter.getAvailableTokens() << ")" << std::endl;
        }
    }
    
    std::cout << "\nSummary: " << allowed << " allowed, " << denied << " denied" << std::endl;
    std::cout << std::endl;
}

void demonstrateRefill() {
    std::cout << "=== Refill Rate Example ===" << std::endl;
    
    // Small bucket: 3 tokens, 1 token per second
    TokenBucket limiter(3.0, 1.0);
    
    std::cout << "Consuming all 3 tokens..." << std::endl;
    for (int i = 0; i < 3; ++i) {
        limiter.tryConsume();
        std::cout << "Consumed token " << i + 1 << ", remaining: " 
                  << limiter.getAvailableTokens() << std::endl;
    }
    
    std::cout << "\nWaiting 2 seconds for tokens to refill..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "Available tokens after 2 seconds: " 
              << limiter.getAvailableTokens() << std::endl;
    
    if (limiter.tryConsume()) {
        std::cout << "Successfully consumed a token after refill!" << std::endl;
    }
    std::cout << std::endl;
}

void demonstrateBurst() {
    std::cout << "=== Burst Capacity Example ===" << std::endl;
    
    // Large capacity allows bursts
    TokenBucket limiter(100.0, 10.0);
    
    std::cout << "Trying to consume 50 tokens at once..." << std::endl;
    if (limiter.tryConsume(50)) {
        std::cout << "Burst of 50 tokens ALLOWED" << std::endl;
        std::cout << "Remaining tokens: " << limiter.getAvailableTokens() << std::endl;
    } else {
        std::cout << "Burst of 50 tokens DENIED" << std::endl;
    }
    
    std::cout << "\nTrying to consume 60 more tokens..." << std::endl;
    if (limiter.tryConsume(60)) {
        std::cout << "Burst of 60 tokens ALLOWED" << std::endl;
    } else {
        std::cout << "Burst of 60 tokens DENIED (only " 
                  << limiter.getAvailableTokens() << " available)" << std::endl;
    }
    std::cout << std::endl;
}

void demonstrateConcurrentAccess() {
    std::cout << "=== Thread Safety Example ===" << std::endl;
    
    TokenBucket limiter(20.0, 5.0);
    std::atomic<int> allowed(0);
    std::atomic<int> denied(0);
    
    auto worker = [&limiter, &allowed, &denied](int id) {
        for (int i = 0; i < 10; ++i) {
            if (limiter.tryConsume()) {
                allowed++;
            } else {
                denied++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };
    
    std::vector<std::thread> threads;
    const int numThreads = 3;
    
    std::cout << "Starting " << numThreads << " threads, each making 10 requests..." << std::endl;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Total allowed: " << allowed.load() << std::endl;
    std::cout << "Total denied: " << denied.load() << std::endl;
    std::cout << std::endl;
}

int main() {
    try {
        demonstrateBasicUsage();
        demonstrateRefill();
        demonstrateBurst();
        demonstrateConcurrentAccess();
        
        std::cout << "All examples completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

