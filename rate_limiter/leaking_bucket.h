#ifndef LEAKING_BUCKET_H
#define LEAKING_BUCKET_H

#include <chrono>
#include <mutex>
#include <queue>

/**
 * Leaking Bucket Rate Limiter
 * 
 * Implements a leaking bucket algorithm for rate limiting:
 * - Requests accumulate in a bucket (queue)
 * - Requests are processed (leaked) at a fixed rate
 * - If the bucket is full, new requests are rejected
 * - Provides smooth, constant output rate
 */
class LeakingBucket {
public:
    /**
     * Constructor
     * @param capacity Maximum number of requests the bucket can hold
     * @param leakRate Requests processed per second
     */
    LeakingBucket(int capacity, double leakRate);
    
    /**
     * Try to add a request to the bucket
     * @return true if request was added (allowed), false if bucket is full (rate limited)
     */
    bool tryAdd();
    
    /**
     * Try to add multiple requests to the bucket
     * @param count Number of requests to add
     * @return true if all requests were added, false otherwise
     */
    bool tryAdd(int count);
    
    /**
     * Get the current number of requests in the bucket
     * @return Number of requests currently queued
     */
    int getQueueSize() const;
    
    /**
     * Get the capacity of the bucket
     * @return Maximum capacity
     */
    int getCapacity() const;
    
    /**
     * Get the leak rate
     * @return Requests processed per second
     */
    double getLeakRate() const;
    
    /**
     * Reset the bucket (clear all queued requests)
     */
    void reset();

private:
    int capacity_;              // Maximum requests
    double leakRate_;           // Requests processed per second
    std::queue<std::chrono::steady_clock::time_point> queue_;  // Queue of request timestamps
    std::chrono::steady_clock::time_point lastLeak_;  // Last time requests were leaked
    mutable std::mutex mutex_;  // Mutex for thread safety
    
    /**
     * Process (leak) requests from the bucket based on elapsed time
     */
    void leak();
};

#endif // LEAKING_BUCKET_H

