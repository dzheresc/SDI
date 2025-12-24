#include "sliding_window_counter.h"
#include <stdexcept>
#include <algorithm>
#include <cmath>

SlidingWindowCounter::SlidingWindowCounter(int maxRequests, int windowSizeSeconds, int numSubWindows)
    : maxRequests_(maxRequests)
    , windowSizeSeconds_(windowSizeSeconds)
    , numSubWindows_(numSubWindows)
    , subWindowSize_(static_cast<double>(windowSizeSeconds) / numSubWindows)
    , subWindowCounts_(numSubWindows, 0)
    , subWindowStarts_(numSubWindows)
{
    if (maxRequests <= 0 || windowSizeSeconds <= 0 || numSubWindows <= 0) {
        throw std::invalid_argument("Max requests, window size, and num sub-windows must be positive");
    }
    
    // Initialize all sub-window start times to current time
    auto now = std::chrono::steady_clock::now();
    for (auto& start : subWindowStarts_) {
        start = now;
    }
}

bool SlidingWindowCounter::tryAllow() {
    return tryAllow(1);
}

bool SlidingWindowCounter::tryAllow(int count) {
    if (count <= 0) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Update sub-windows and get current count
    double currentCount = updateAndGetCount();
    
    // Check if we have capacity for the requests
    if (currentCount + count <= maxRequests_) {
        // Add to the current sub-window
        int currentIndex = getCurrentSubWindowIndex();
        subWindowCounts_[currentIndex] += count;
        return true;
    }
    
    return false;
}

double SlidingWindowCounter::getCurrentCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Create a non-const reference to call updateAndGetCount
    return const_cast<SlidingWindowCounter*>(this)->updateAndGetCount();
}

int SlidingWindowCounter::getMaxRequests() const {
    return maxRequests_;
}

int SlidingWindowCounter::getWindowSizeSeconds() const {
    return windowSizeSeconds_;
}

int SlidingWindowCounter::getNumSubWindows() const {
    return numSubWindows_;
}

void SlidingWindowCounter::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::fill(subWindowCounts_.begin(), subWindowCounts_.end(), 0);
    
    auto now = std::chrono::steady_clock::now();
    for (auto& start : subWindowStarts_) {
        start = now;
    }
}

double SlidingWindowCounter::updateAndGetCount() {
    auto now = std::chrono::steady_clock::now();
    auto windowStart = now - std::chrono::seconds(windowSizeSeconds_);
    
    // Calculate weighted count across all sub-windows
    double totalCount = 0.0;
    
    for (int i = 0; i < numSubWindows_; ++i) {
        auto subWindowStart = subWindowStarts_[i];
        
        // If sub-window is completely outside the main window, reset it
        if (subWindowStart < windowStart) {
            subWindowCounts_[i] = 0;
            // Set to a time that will be used for the next request
            subWindowStarts_[i] = now;
            continue;
        }
        
        // Calculate how much of this sub-window overlaps with the main window
        // The weight is based on how much of the sub-window is still within the main window
        auto subWindowEnd = subWindowStart + std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::duration<double>(subWindowSize_)
        );
        
        // Calculate overlap: how much of this sub-window is within the main window
        auto overlapStart = std::max(subWindowStart, windowStart);
        auto overlapEnd = std::min(subWindowEnd, now);
        
        if (overlapEnd > overlapStart) {
            auto overlapDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                overlapEnd - overlapStart
            ).count() / 1e9;
            double weight = overlapDuration / subWindowSize_;
            totalCount += subWindowCounts_[i] * weight;
        }
    }
    
    return totalCount;
}

int SlidingWindowCounter::getCurrentSubWindowIndex() const {
    auto now = std::chrono::steady_clock::now();
    
    // Find the sub-window that should receive the current request
    // We use a circular buffer approach based on time
    // Calculate which sub-window slot we're in based on current time
    
    // Use the first sub-window start as a reference point
    auto referenceTime = subWindowStarts_[0];
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now - referenceTime
    ).count() / 1e9;  // Convert to seconds
    
    // Calculate which sub-window index we should use (circular)
    int subWindowIndex = static_cast<int>(std::floor(elapsed / subWindowSize_)) % numSubWindows_;
    
    // If this sub-window is too old, reset it and use it for the new request
    auto subWindowStart = subWindowStarts_[subWindowIndex];
    auto windowStart = now - std::chrono::seconds(windowSizeSeconds_);
    
    if (subWindowStart < windowStart) {
        // This sub-window has expired, reset it
        subWindowCounts_[subWindowIndex] = 0;
        subWindowStarts_[subWindowIndex] = now;
    }
    
    return subWindowIndex;
}

