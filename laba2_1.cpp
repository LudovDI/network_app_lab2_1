#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <WinSock2.h>

#include <string>
#include <iostream>
#include <filesystem>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 6789
#define BUFFER_SIZE 1024

void handle_client(SOCKET clientSocket) {
    try {
        char buffer[BUFFER_SIZE];
        int bytes_received = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytes_received == SOCKET_ERROR) {
            throw "Error receiving data from client";
        }

        std::string message(buffer);
        std::string filename = message.substr(message.find(" ") + 1);
        filename = filename.substr(0, filename.find(" "));
        std::ifstream file(filename.substr(1));

        if (!file.is_open()) {
            std::string not_found_response = "HTTP/1.1 404 Not Found\r\n\r\nFile Not Found";
            send(clientSocket, not_found_response.c_str(), not_found_response.size(), 0);
        }
        else {
            std::string response_header = "HTTP/1.1 200 OK\r\n\r\n";
            send(clientSocket, response_header.c_str(), response_header.size(), 0);
            std::string line;
            while (std::getline(file, line)) {
                send(clientSocket, line.c_str(), line.size(), 0);
            }
            file.close();
        }

        closesocket(clientSocket);
    }
    catch (...) {
        closesocket(clientSocket);
    }
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int sin_size = sizeof(clientAddr);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error binding to socket" << std::endl;
        return 1;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Error listening on socket" << std::endl;
        return 1;
    }

    std::cout << "Server is ready to accept connections." << std::endl;

    while (true) {
        if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &sin_size)) == INVALID_SOCKET) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        std::cout << "Connection established with " << inet_ntoa(clientAddr.sin_addr) << std::endl;

        std::thread client_thread(handle_client, clientSocket);
        client_thread.detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}