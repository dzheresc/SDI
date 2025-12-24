#include "kv_store.h"
#include <iostream>
#include "test_kv_store.cpp"

int main() {
    try {
        std::cout << "========================================" << std::endl;
        std::cout << "  Distributed Key-Value Store Test" << std::endl;
        std::cout << "========================================\n" << std::endl;
        
        runAllTests();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

