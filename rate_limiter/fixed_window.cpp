#include "fixed_window.h"
#include <stdexcept>
#include <algorithm>

FixedWindow::FixedWindow(int maxRequests, int windowSizeSeconds)
    : maxRequests_(maxRequests)
    , windowSizeSeconds_(windowSizeSeconds)
    , currentCount_(0)
    , windowStart_(std::chrono::steady_clock::now())
{
    if (maxRequests <= 0 || windowSizeSeconds <= 0) {
        throw std::invalid_argument("Max requests and window size must be positive");
    }
}

bool FixedWindow::tryAllow() {
    return tryAllow(1);
}

bool FixedWindow::tryAllow(int count) {
    if (count <= 0) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if we need to start a new window
    updateWindow();
    
    // Check if we have capacity for the requests
    if (currentCount_ + count <= maxRequests_) {
        currentCount_ += count;
        return true;
    }
    
    return false;
}

int FixedWindow::getCurrentCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Create a non-const reference to call updateWindow
    const_cast<FixedWindow*>(this)->updateWindow();
    
    return currentCount_;
}

int FixedWindow::getMaxRequests() const {
    return maxRequests_;
}

int FixedWindow::getWindowSizeSeconds() const {
    return windowSizeSeconds_;
}

double FixedWindow::getTimeRemainingInWindow() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now - windowStart_
    ).count() / 1e9;  // Convert to seconds
    
    double remaining = windowSizeSeconds_ - elapsed;
    return std::max(0.0, remaining);
}

void FixedWindow::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    currentCount_ = 0;
    windowStart_ = std::chrono::steady_clock::now();
}

void FixedWindow::updateWindow() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - windowStart_
    ).count();
    
    // If we've moved to a new window, reset the counter
    if (elapsed >= windowSizeSeconds_) {
        currentCount_ = 0;
        windowStart_ = now;
    }
}

