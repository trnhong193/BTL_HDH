#include "file_manager.h"
#include "checksum_utils.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h>
#include <fcntl.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define CHUNK_SIZE 1024

void* handle_client(void* client_sock) {
    int sock = *(int*)client_sock;
    char buffer[1024];
    int n;

    while ((n = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        buffer[n] = '\0';
        std::cout << "Received request: " << buffer << std::endl;

        if (strcmp(buffer, "LIST") == 0) {
            list_files(sock);
        } else if (strncmp(buffer, "UPLOAD", 6) == 0) {
            upload_file(sock, buffer);
        } else if (strncmp(buffer, "DOWNLOAD", 8) == 0) {
            download_file(sock, buffer);
        } else {
            std::string msg = "Invalid command\n";
            send(sock, msg.c_str(), msg.size(), 0);
        }
    }

    close(sock);
    return NULL;
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t thread_id;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    listen(server_sock, MAX_CLIENTS);
    std::cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        std::cout << "Client connected: " << inet_ntoa(client_addr.sin_addr) << std::endl;

        if (pthread_create(&thread_id, NULL, handle_client, (void*)&client_sock) != 0) {
            perror("Thread creation failed");
        }
    }

    close(server_sock);
    return 0;
}
