#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "file_transfer.h"
#include "checksum_utils.h"

#define SERVER_IP "192.168.0.129"
#define SERVER_PORT 18080
#define CHUNK_SIZE 1000

void print_menu() {
    std::cout << "\nClient Menu:\n";
    std::cout << "1. List files on server\n";
    std::cout << "2. Upload file to server\n";
    std::cout << "3. Download file from server\n";
    std::cout << "4. Exit\n";
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[CHUNK_SIZE];
    int choice;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Configure server information
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        exit(1);
    }

    while (true) {
        print_menu();
        std::cout << "Enter your choice: ";
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
            case 1:
                list_files(sock);
                break;
            case 2:
                upload_file(sock);
                break;
            case 3:
                download_file(sock);
                break;
            case 4:
                std::cout << "Exiting...\n";
                close(sock);
                return 0;
            default:
                std::cout << "Invalid choice, please try again.\n";
                break;
        }
    }

    return 0;
}
