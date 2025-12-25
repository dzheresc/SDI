#include "url_shortener.h"
#include <iostream>
#include "test_url_shortener.cpp"

int main() {
    try {
        runAllTests();
        
        return (testsFailed == 0) ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

