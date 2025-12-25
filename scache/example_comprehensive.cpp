#include "scache.h"
#include <iostream>
#include "test_strings_cache_comprehensive.cpp"

int main() {
    try {
        runAllComprehensiveTests();
        
        return (testsFailed == 0) ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

