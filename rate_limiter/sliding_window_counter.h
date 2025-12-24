#ifndef SLIDING_WINDOW_COUNTER_H
#define SLIDING_WINDOW_COUNTER_H

#include <chrono>
#include <mutex>
#include <vector>

/**
 * Sliding Window Counter Rate Limiter
 * 
 * Implements a sliding window counter algorithm for rate limiting:
 * - Divides the window into smaller sub-windows
 * - Maintains counters for each sub-window
 * - Calculates weighted count across sub-windows
 * - More memory efficient than sliding window log, with slight approximation
 */
class SlidingWindowCounter {
public:
    /**
     * Constructor
     * @param maxRequests Maximum number of requests allowed in the window
     * @param windowSizeSeconds Size of the sliding window in seconds
     * @param numSubWindows Number of sub-windows to divide the main window into (default: 10)
     */
    SlidingWindowCounter(int maxRequests, int windowSizeSeconds, int numSubWindows = 10);
    
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
     * Get the current estimated count in the sliding window
     * @return Estimated number of requests in current window
     */
    double getCurrentCount() const;
    
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
     * Get the number of sub-windows
     * @return Number of sub-windows
     */
    int getNumSubWindows() const;
    
    /**
     * Reset all counters (clear the window)
     */
    void reset();

private:
    int maxRequests_;           // Maximum requests in window
    int windowSizeSeconds_;     // Window size in seconds
    int numSubWindows_;         // Number of sub-windows
    double subWindowSize_;      // Size of each sub-window in seconds
    std::vector<int> subWindowCounts_;  // Counters for each sub-window
    std::vector<std::chrono::steady_clock::time_point> subWindowStarts_;  // Start time of each sub-window
    mutable std::mutex mutex_;  // Mutex for thread safety
    
    /**
     * Update sub-windows and calculate current count
     * @return Current weighted count across all sub-windows
     */
    double updateAndGetCount();
    
    /**
     * Get the index of the current sub-window based on time
     */
    int getCurrentSubWindowIndex() const;
};

#endif // SLIDING_WINDOW_COUNTER_H

