#ifndef TOKEN_BUCKET_H
#define TOKEN_BUCKET_H

#include <chrono>
#include <mutex>
#include <atomic>

/**
 * Token Bucket Rate Limiter
 * 
 * Implements a token bucket algorithm for rate limiting:
 * - Tokens are added to the bucket at a fixed refill rate
 * - Each request consumes one token
 * - Requests are allowed if tokens are available, otherwise rate limited
 */
class TokenBucket {
public:
    /**
     * Constructor
     * @param capacity Maximum number of tokens the bucket can hold
     * @param refillRate Tokens added per second
     */
    TokenBucket(double capacity, double refillRate);
    
    /**
     * Try to consume a token
     * @return true if token was consumed (request allowed), false otherwise (rate limited)
     */
    bool tryConsume();
    
    /**
     * Try to consume multiple tokens
     * @param tokens Number of tokens to consume
     * @return true if all tokens were consumed, false otherwise
     */
    bool tryConsume(int tokens);
    
    /**
     * Get the current number of available tokens
     * @return Number of tokens currently available
     */
    double getAvailableTokens() const;
    
    /**
     * Get the capacity of the bucket
     * @return Maximum capacity
     */
    double getCapacity() const;
    
    /**
     * Get the refill rate
     * @return Tokens per second
     */
    double getRefillRate() const;
    
    /**
     * Reset the bucket to full capacity
     */
    void reset();

private:
    double capacity_;           // Maximum tokens
    double refillRate_;         // Tokens per second
    double tokens_;             // Current token count
    std::chrono::steady_clock::time_point lastRefill_;  // Last time tokens were refilled
    mutable std::mutex mutex_;  // Mutex for thread safety
    
    /**
     * Refill tokens based on elapsed time
     */
    void refill();
};

#endif // TOKEN_BUCKET_H

