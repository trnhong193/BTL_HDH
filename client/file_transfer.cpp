#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <filesystem>
#include "file_transfer.h" // Chứa các khai báo hàm

#define MAX_BUFFER_SIZE 1024

// Hàm liệt kê các file từ server
void list_files(int sock) {
    char buffer[MAX_BUFFER_SIZE];

    // Gửi yêu cầu LIST đến server
    send(sock, "LIST", 4, 0);

    // Nhận và hiển thị danh sách file từ server
    int bytes_received = recv(sock, buffer, MAX_BUFFER_SIZE, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0'; // Đảm bảo kết thúc chuỗi
        std::cout << "Files on server:\n" << buffer << std::endl;
    } else {
        std::cout << "Failed to receive file list.\n";
    }
}

// Hàm upload file từ client lên server
void upload_file(int sock) {
    std::string filepath;
    char buffer[MAX_BUFFER_SIZE];
    std::ifstream infile;

    std::cout << "Enter the filename to upload: ";
    std::getline(std::cin, filepath);

    // Mở file để đọc
    infile.open(filepath, std::ios::binary);
    if (!infile) {
        std::cerr << "Error: File does not exist or cannot be opened.\n";
        return;
    }

    // Tách tên file từ đường dẫn đầy đủ
    std::string filename = std::filesystem::path(filepath).filename();

    // Gửi yêu cầu upload đến server
    std::string command = "UPLOAD " + filename;
    if (send(sock, command.c_str(), command.length(), 0) == -1) {
        std::cerr << "Error: Failed to send upload command.\n";
        infile.close();
        return;
    }

    // Đọc file và gửi dữ liệu theo chunk
    while (infile.read(buffer, MAX_BUFFER_SIZE)) {
        if (send(sock, buffer, infile.gcount(), 0) == -1) {
            std::cerr << "Error: Failed to send file data.\n";
            infile.close();
            return;
        }
    }

    // Gửi phần còn lại của file (nếu có)
    if (infile.gcount() > 0) {
        if (send(sock, buffer, infile.gcount(), 0) == -1) {
            std::cerr << "Error: Failed to send remaining file data.\n";
            infile.close();
            return;
        }
    }

    // Gửi tín hiệu kết thúc file
    if (send(sock, "EOF", 3, 0) == -1) {
        std::cerr << "Error: Failed to send EOF signal.\n";
    } else {
        std::cout << "Upload complete.\n";
    }

    infile.close();
}

// Hàm download file từ server
void download_file(int sock) {
    std::string filename;
    char buffer[MAX_BUFFER_SIZE];
    std::ofstream outfile;

    std::cout << "Enter the filename to download: ";
    std::getline(std::cin, filename);

    // Gửi yêu cầu download đến server
    std::string command = "DOWNLOAD " + filename;
    send(sock, command.c_str(), command.length(), 0);

    // Mở file để ghi
    outfile.open(filename, std::ios::binary);
    if (!outfile) {
        std::cerr << "Error: Cannot create file for writing.\n";
        return;
    }

    // Nhận và ghi dữ liệu từ server
    int bytes_received;
    while ((bytes_received = recv(sock, buffer, MAX_BUFFER_SIZE, 0)) > 0) {
        if (bytes_received == 3 && strncmp(buffer, "EOF", 3) == 0) {
            break;
        }
        outfile.write(buffer, bytes_received);
    }

    std::cout << "Download complete.\n";
    outfile.close();
}
