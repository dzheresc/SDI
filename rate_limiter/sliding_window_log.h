#ifndef SLIDING_WINDOW_LOG_H
#define SLIDING_WINDOW_LOG_H

#include <chrono>
#include <mutex>
#include <deque>

/**
 * Sliding Window Log Rate Limiter
 * 
 * Implements a sliding window log algorithm for rate limiting:
 * - Maintains a log of request timestamps
 * - Removes timestamps older than the window size
 * - Allows requests if count in current window is below limit
 * - Provides accurate rate limiting with true sliding window behavior
 */
class SlidingWindowLog {
public:
    /**
     * Constructor
     * @param maxRequests Maximum number of requests allowed in the window
     * @param windowSizeSeconds Size of the sliding window in seconds
     */
    SlidingWindowLog(int maxRequests, int windowSizeSeconds);
    
    /**
     * Try to allow a request
     * @return true if request was allowed, false if rate limited
     */
    bool tryAllow();
    
    /**
     * Try to allow multiple requests
     * @param count Number of requests to allow
     * @return true if all requests were allowed, false otherwise
     */
    bool tryAllow(int count);
    
    /**
     * Get the current number of requests in the sliding window
     * @return Number of requests in current window
     */
    int getCurrentCount() const;
    
    /**
     * Get the maximum requests allowed in the window
     * @return Maximum requests per window
     */
    int getMaxRequests() const;
    
    /**
     * Get the window size in seconds
     * @return Window size in seconds
     */
    int getWindowSizeSeconds() const;
    
    /**
     * Get the time until the oldest request expires (in seconds)
     * @return Seconds until oldest request expires, or 0 if no requests
     */
    double getTimeUntilOldestExpires() const;
    
    /**
     * Clear all request logs (reset)
     */
    void reset();

private:
    int maxRequests_;           // Maximum requests in window
    int windowSizeSeconds_;     // Window size in seconds
    std::deque<std::chrono::steady_clock::time_point> requestLog_;  // Log of request timestamps
    mutable std::mutex mutex_;  // Mutex for thread safety
    
    /**
     * Remove expired timestamps from the log (older than window size)
     */
    void removeExpiredRequests();
};

#endif // SLIDING_WINDOW_LOG_H

