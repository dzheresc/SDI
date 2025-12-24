#include <iostream>

// Forward declarations for test functions
void runAllTokenBucketTests();
void runAllLeakingBucketTests();
void runAllFixedWindowTests();
void runAllSlidingWindowLogTests();
void runAllSlidingWindowCounterTests();

int main() {
    try {
        std::cout << "========================================" << std::endl;
        std::cout << "  Rate Limiter Test Suite" << std::endl;
        std::cout << "========================================\n" << std::endl;
        
        // Run Token Bucket tests
        std::cout << "\n[TOKEN BUCKET TESTS]\n" << std::endl;
        runAllTokenBucketTests();
        
        std::cout << "\n========================================\n" << std::endl;
        
        // Run Leaking Bucket tests
        std::cout << "\n[LEAKING BUCKET TESTS]\n" << std::endl;
        runAllLeakingBucketTests();
        
        std::cout << "\n========================================\n" << std::endl;
        
        // Run Fixed Window tests
        std::cout << "\n[FIXED WINDOW TESTS]\n" << std::endl;
        runAllFixedWindowTests();
        
        std::cout << "\n========================================\n" << std::endl;
        
        // Run Sliding Window Log tests
        std::cout << "\n[SLIDING WINDOW LOG TESTS]\n" << std::endl;
        runAllSlidingWindowLogTests();
        
        std::cout << "\n========================================\n" << std::endl;
        
        // Run Sliding Window Counter tests
        std::cout << "\n[SLIDING WINDOW COUNTER TESTS]\n" << std::endl;
        runAllSlidingWindowCounterTests();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "All tests completed successfully!" << std::endl;
        std::cout << "========================================" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

