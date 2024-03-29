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

void handle_client(SOCKET my_client_socket) {
    try {
        char buffer[BUFFER_SIZE];
        int bytes_received = recv(my_client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received == SOCKET_ERROR) {
            throw "Error receiving data from client";
        }

        std::string message(buffer);
        std::string filename = message.substr(message.find(" ") + 1);
        filename = filename.substr(0, filename.find(" "));
        std::ifstream file(filename.substr(1));

        if (!file.is_open()) {
            std::string not_found_response = "HTTP/1.1 404 Not Found\r\n\r\nFile Not Found";
            send(my_client_socket, not_found_response.c_str(), not_found_response.size(), 0);
        }
        else {
            std::string response_header = "HTTP/1.1 200 OK\r\n\r\n";
            send(my_client_socket, response_header.c_str(), response_header.size(), 0);
            std::string line;
            while (std::getline(file, line)) {
                send(my_client_socket, line.c_str(), line.size(), 0);
            }
            file.close();
        }

        closesocket(my_client_socket);
    }
    catch (...) {
        closesocket(my_client_socket);
    }
}

int main() {
    WSADATA wsa_data;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int sin_size = sizeof(client_addr);

    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Error binding to socket" << std::endl;
        return 1;
    }

    if (listen(server_socket, 5) == SOCKET_ERROR) {
        std::cerr << "Error listening on socket" << std::endl;
        return 1;
    }

    std::cout << "Server is ready to accept connections." << std::endl;

    while (true) {
        if ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &sin_size)) == INVALID_SOCKET) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        std::cout << "Connection established with " << inet_ntoa(client_addr.sin_addr) << std::endl;

        std::thread client_thread(handle_client, client_socket);
        client_thread.detach();
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}