#include "leaking_bucket.h"
#include <stdexcept>
#include <cmath>

LeakingBucket::LeakingBucket(int capacity, double leakRate)
    : capacity_(capacity)
    , leakRate_(leakRate)
    , lastLeak_(std::chrono::steady_clock::now())
{
    if (capacity <= 0 || leakRate <= 0) {
        throw std::invalid_argument("Capacity and leak rate must be positive");
    }
}

bool LeakingBucket::tryAdd() {
    return tryAdd(1);
}

bool LeakingBucket::tryAdd(int count) {
    if (count <= 0) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Process requests from the bucket based on elapsed time
    leak();
    
    // Check if we have space for all requests
    int availableSpace = capacity_ - static_cast<int>(queue_.size());
    
    if (availableSpace >= count) {
        // Add requests to the queue with current timestamp
        auto now = std::chrono::steady_clock::now();
        for (int i = 0; i < count; ++i) {
            queue_.push(now);
        }
        return true;
    }
    
    return false;
}

int LeakingBucket::getQueueSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Create a non-const reference to call leak
    const_cast<LeakingBucket*>(this)->leak();
    
    return static_cast<int>(queue_.size());
}

int LeakingBucket::getCapacity() const {
    return capacity_;
}

double LeakingBucket::getLeakRate() const {
    return leakRate_;
}

void LeakingBucket::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    while (!queue_.empty()) {
        queue_.pop();
    }
    lastLeak_ = std::chrono::steady_clock::now();
}

void LeakingBucket::leak() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now - lastLeak_
    ).count() / 1e9;  // Convert to seconds
    
    if (elapsed > 0 && !queue_.empty()) {
        // Calculate how many requests should be processed
        double requestsToProcess = elapsed * leakRate_;
        int requestsToRemove = static_cast<int>(std::floor(requestsToProcess));
        
        // Remove processed requests from the queue
        for (int i = 0; i < requestsToRemove && !queue_.empty(); ++i) {
            queue_.pop();
        }
        
        lastLeak_ = now;
    }
}

