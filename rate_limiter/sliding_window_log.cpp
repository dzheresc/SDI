#include "sliding_window_log.h"
#include <stdexcept>
#include <algorithm>

SlidingWindowLog::SlidingWindowLog(int maxRequests, int windowSizeSeconds)
    : maxRequests_(maxRequests)
    , windowSizeSeconds_(windowSizeSeconds)
{
    if (maxRequests <= 0 || windowSizeSeconds <= 0) {
        throw std::invalid_argument("Max requests and window size must be positive");
    }
}

bool SlidingWindowLog::tryAllow() {
    return tryAllow(1);
}

bool SlidingWindowLog::tryAllow(int count) {
    if (count <= 0) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Remove expired requests (older than window size)
    removeExpiredRequests();
    
    // Check if we have capacity for the requests
    if (static_cast<int>(requestLog_.size()) + count <= maxRequests_) {
        // Add current timestamp(s) to the log
        auto now = std::chrono::steady_clock::now();
        for (int i = 0; i < count; ++i) {
            requestLog_.push_back(now);
        }
        return true;
    }
    
    return false;
}

int SlidingWindowLog::getCurrentCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Create a non-const reference to call removeExpiredRequests
    const_cast<SlidingWindowLog*>(this)->removeExpiredRequests();
    
    return static_cast<int>(requestLog_.size());
}

int SlidingWindowLog::getMaxRequests() const {
    return maxRequests_;
}

int SlidingWindowLog::getWindowSizeSeconds() const {
    return windowSizeSeconds_;
}

double SlidingWindowLog::getTimeUntilOldestExpires() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (requestLog_.empty()) {
        return 0.0;
    }
    
    // Create a non-const reference to call removeExpiredRequests
    const_cast<SlidingWindowLog*>(this)->removeExpiredRequests();
    
    if (requestLog_.empty()) {
        return 0.0;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto oldest = requestLog_.front();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now - oldest
    ).count() / 1e9;  // Convert to seconds
    
    double remaining = windowSizeSeconds_ - elapsed;
    return std::max(0.0, remaining);
}

void SlidingWindowLog::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    requestLog_.clear();
}

void SlidingWindowLog::removeExpiredRequests() {
    if (requestLog_.empty()) {
        return;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto windowStart = now - std::chrono::seconds(windowSizeSeconds_);
    
    // Remove all timestamps older than the window
    while (!requestLog_.empty() && requestLog_.front() < windowStart) {
        requestLog_.pop_front();
    }
}

