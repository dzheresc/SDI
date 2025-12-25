#include "url_shortener_kv.h"
#include <iostream>
#include "test_url_shortener_kv.cpp"

int main() {
    try {
        runAllKVTests();
        
        return (testsFailed == 0) ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

