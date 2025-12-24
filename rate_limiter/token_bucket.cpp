#include "token_bucket.h"
#include <algorithm>
#include <stdexcept>

TokenBucket::TokenBucket(double capacity, double refillRate)
    : capacity_(capacity)
    , refillRate_(refillRate)
    , tokens_(capacity)  // Start with full bucket
    , lastRefill_(std::chrono::steady_clock::now())
{
    if (capacity <= 0 || refillRate <= 0) {
        throw std::invalid_argument("Capacity and refill rate must be positive");
    }
}

bool TokenBucket::tryConsume() {
    return tryConsume(1);
}

bool TokenBucket::tryConsume(int tokens) {
    if (tokens <= 0) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Refill tokens based on elapsed time
    refill();
    
    // Check if we have enough tokens
    if (tokens_ >= tokens) {
        tokens_ -= tokens;
        return true;
    }
    
    return false;
}

double TokenBucket::getAvailableTokens() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Create a non-const reference to call refill
    const_cast<TokenBucket*>(this)->refill();
    
    return tokens_;
}

double TokenBucket::getCapacity() const {
    return capacity_;
}

double TokenBucket::getRefillRate() const {
    return refillRate_;
}

void TokenBucket::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    tokens_ = capacity_;
    lastRefill_ = std::chrono::steady_clock::now();
}

void TokenBucket::refill() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now - lastRefill_
    ).count() / 1e9;  // Convert to seconds
    
    if (elapsed > 0) {
        // Add tokens based on elapsed time and refill rate
        double tokensToAdd = elapsed * refillRate_;
        tokens_ = std::min(capacity_, tokens_ + tokensToAdd);
        lastRefill_ = now;
    }
}

