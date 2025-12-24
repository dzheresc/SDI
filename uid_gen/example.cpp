#include "snowflake_id.h"
#include <iostream>
#include "test_snowflake.cpp"

int main() {
    try {
        std::cout << "========================================" << std::endl;
        std::cout << "  Snowflake ID Generator Test Suite" << std::endl;
        std::cout << "========================================\n" << std::endl;
        
        runAllTests();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

