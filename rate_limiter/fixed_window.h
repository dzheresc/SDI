#ifndef FIXED_WINDOW_H
#define FIXED_WINDOW_H

#include <chrono>
#include <mutex>

/**
 * Fixed Window Counter Rate Limiter
 * 
 * Implements a fixed window counter algorithm for rate limiting:
 * - Divides time into fixed windows (e.g., 1 second, 1 minute)
 * - Counts requests within the current window
 * - Allows requests if count is below the limit
 * - Resets counter at the start of each new window
 */
class FixedWindow {
public:
    /**
     * Constructor
     * @param maxRequests Maximum number of requests allowed per window
     * @param windowSizeSeconds Size of each window in seconds
     */
    FixedWindow(int maxRequests, int windowSizeSeconds);
    
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
     * Get the current number of requests in the current window
     * @return Number of requests in current window
     */
    int getCurrentCount() const;
    
    /**
     * Get the maximum requests allowed per window
     * @return Maximum requests per window
     */
    int getMaxRequests() const;
    
    /**
     * Get the window size in seconds
     * @return Window size in seconds
     */
    int getWindowSizeSeconds() const;
    
    /**
     * Get the time remaining in the current window (in seconds)
     * @return Seconds until next window starts
     */
    double getTimeRemainingInWindow() const;
    
    /**
     * Reset the counter (start a new window immediately)
     */
    void reset();

private:
    int maxRequests_;           // Maximum requests per window
    int windowSizeSeconds_;     // Window size in seconds
    int currentCount_;          // Current request count
    std::chrono::steady_clock::time_point windowStart_;  // Start time of current window
    mutable std::mutex mutex_;  // Mutex for thread safety
    
    /**
     * Check if we're in a new window and reset if necessary
     */
    void updateWindow();
};

#endif // FIXED_WINDOW_H

