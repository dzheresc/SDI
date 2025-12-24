#include "consistent_hash.h"
#include <iostream>
#include "test_consistent_hash.cpp"

int main() {
    try {
        std::cout << "========================================" << std::endl;
        std::cout << "  Consistent Hashing Test Suite" << std::endl;
        std::cout << "========================================\n" << std::endl;
        
        runAllTests();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

