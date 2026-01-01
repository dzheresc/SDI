#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

const int UDP_PORT = 5555;
const int BUFFER_SIZE = 4096;

int main() {
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];
    int result;

    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed with error: " << result << std::endl;
        return 1;
    }

    // Create UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Setup server address structure
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces
    serverAddr.sin_port = htons(UDP_PORT);

    // Bind socket to port
    result = bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "UDP listener started on port " << UDP_PORT << std::endl;
    std::cout << "Waiting for data... (Press Ctrl+C to exit)" << std::endl;
    std::cout << std::endl;

    // Main receive loop
    while (true) {
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        
        // Receive data
        int bytesReceived = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0,
                                     (struct sockaddr*)&clientAddr, &clientAddrLen);
        
        if (bytesReceived == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error != WSAECONNRESET) {
                std::cerr << "recvfrom failed with error: " << error << std::endl;
            }
            continue;
        }

        // Null-terminate the received data
        buffer[bytesReceived] = '\0';

        // Print sender information
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
        std::cout << "Received " << bytesReceived << " bytes from " 
                  << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;

        // Print received data
        std::cout << "Data: ";
        
        // Try to print as string first
        bool isPrintable = true;
        for (int i = 0; i < bytesReceived; i++) {
            if (!isprint(buffer[i]) && buffer[i] != '\n' && buffer[i] != '\r' && buffer[i] != '\t') {
                isPrintable = false;
                break;
            }
        }

        if (isPrintable) {
            std::cout << buffer << std::endl;
        } else {
            // Print as hex if not printable
            std::cout << "[Hex: ";
            for (int i = 0; i < bytesReceived; i++) {
                std::cout << std::hex << std::uppercase 
                         << (static_cast<unsigned char>(buffer[i]) < 16 ? "0" : "")
                         << static_cast<unsigned int>(static_cast<unsigned char>(buffer[i])) 
                         << " ";
            }
            std::cout << std::dec << "]" << std::endl;
        }
        std::cout << std::endl;
    }

    // Cleanup (this code is unreachable in the current loop, but good practice)
    closesocket(sock);
    WSACleanup();
    return 0;
}

