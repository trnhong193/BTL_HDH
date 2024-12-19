#include <iostream>
#include <cstring>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <filesystem>

#define MAX_BUFFER_SIZE 1024

// Hàm đệ quy liệt kê các file và thư mục trong thư mục hiện tại
void list_files_recursive(const std::string& dir_path, int sock) {
    DIR* dir = opendir(dir_path.c_str());
    if (dir == nullptr) {
        std::string msg = "Error opening directory: " + dir_path + "\n";
        send(sock, msg.c_str(), msg.size(), 0);
        return;
    }

    struct dirent* entry;
    struct stat entry_stat;

    while ((entry = readdir(dir)) != nullptr) {
        std::string entry_name = entry->d_name;

        // Bỏ qua thư mục "." và ".."
        if (entry_name == "." || entry_name == "..") {
            continue;
        }

        std::string full_path = dir_path + "/" + entry_name;

        // Kiểm tra xem entry có phải là thư mục không
        if (stat(full_path.c_str(), &entry_stat) == 0) {
            if (S_ISDIR(entry_stat.st_mode)) {
                // Nếu là thư mục, gửi thông tin thư mục và gọi đệ quy để liệt kê
                std::string msg = "DIR: " + full_path + "\n";
                send(sock, msg.c_str(), msg.size(), 0);
                list_files_recursive(full_path, sock);
            } else {
                // Nếu là file, gửi thông tin file
                std::string msg = "FILE: " + full_path + "\n";
                send(sock, msg.c_str(), msg.size(), 0);
            }
        }
    }

    closedir(dir);
}

// Hàm xử lý yêu cầu liệt kê file
void list_files(int sock) {
    std::string root_dir = "/home/trnhong193/Documents/BTL_HDH_20241/uploads";  // Thư mục chứa các file cần liệt kê
    list_files_recursive(root_dir, sock);
}

void upload_file(int sock, const char* command) {
    char buffer[MAX_BUFFER_SIZE];

    // Trích xuất tên file từ command
    std::string filename = std::string(command).substr(7);  // Bỏ "UPLOAD "
    std::string save_path = "/home/trnhong193/Documents/BTL_HDH_20241/uploads/" + filename;

    std::ofstream outfile(save_path, std::ios::binary);
    if (!outfile) {
        perror("Error: Cannot save file on server.");
        return;
    }

    // Nhận dữ liệu từ client và ghi vào file
    int bytes_received;
    while ((bytes_received = recv(sock, buffer, MAX_BUFFER_SIZE, 0)) > 0) {
        // Kiểm tra tín hiệu kết thúc
        if (bytes_received == 3 && strncmp(buffer, "EOF", 3) == 0) {
            break;
        }
        outfile.write(buffer, bytes_received);
    }

    if (!outfile) {
        std::cerr << "Error: Failed to write to file.\n";
    } else {
        std::cout << "File received successfully: " << save_path << "\n";
    }

    outfile.close();
}


// Hàm upload file từ client
// void upload_file(int sock, const char* command) {
//     char buffer[MAX_BUFFER_SIZE];

//     // Trích xuất tên file từ command
//     std::string filename = std::string(command).substr(7);  // Bỏ "UPLOAD "
//     std::string save_path = "/home/trnhong193/Documents/BTL_HDH_20241/uploads/" + filename;        // Đường dẫn lưu file trên server

//     std::ofstream outfile(save_path, std::ios::binary);
//     if (!outfile) {
//         perror("Error: Cannot save file on server.");
//         return;
//     }

//     int bytes_received;
//     while ((bytes_received = recv(sock, buffer, MAX_BUFFER_SIZE, 0)) > 0) {
//         outfile.write(buffer, bytes_received);
//     }

//     std::string msg = "Upload complete.\n";
//     send(sock, msg.c_str(), msg.size(), 0);
//     outfile.close();
// }


void download_file(int sock, const char* command) {
    char buffer[MAX_BUFFER_SIZE];

    // Trích xuất tên file từ command
    std::string filename = std::string(command).substr(9);  // Bỏ "DOWNLOAD "
    std::string file_path = "/home/trnhong193/Documents/BTL_HDH_20241/uploads/" + filename;

    std::ifstream infile(file_path, std::ios::binary);
    if (!infile) {
        std::string msg = "File not found.\n";
        send(sock, msg.c_str(), msg.size(), 0);
        return;
    }

    // Đọc file và gửi dữ liệu qua socket
    while (infile.read(buffer, MAX_BUFFER_SIZE)) {
        send(sock, buffer, infile.gcount(), 0);
    }
    if (infile.gcount() > 0) { // Gửi phần còn lại nếu có
        send(sock, buffer, infile.gcount(), 0);
    }

    // Gửi tín hiệu kết thúc file
    send(sock, "EOF", 3, 0);

    infile.close();
}

// // Hàm download file cho client
// void download_file(int sock, const char* command) {
//     char buffer[MAX_BUFFER_SIZE];

//     // Trích xuất tên file từ command
//     std::string filename = std::string(command).substr(9);  // Bỏ "DOWNLOAD "
//     std::string file_path = "/home/trnhong193/Documents/BTL_HDH_20241/uploads/" + filename;        // Đường dẫn file trên server

//     std::ifstream infile(file_path, std::ios::binary);
//     if (!infile) {
//         std::string msg = "File not found.\n";
//         send(sock, msg.c_str(), msg.size(), 0);
//         return;
//     }

//     // Đọc file và gửi dữ liệu qua socket
//     while (infile.read(buffer, MAX_BUFFER_SIZE)) {
//         send(sock, buffer, infile.gcount(), 0);
//     }
//     if (infile.gcount() > 0) {
//         send(sock, buffer, infile.gcount(), 0);
//     }

//     std::string msg = "Download complete.\n";
//     send(sock, msg.c_str(), msg.size(), 0);
//     infile.close();
// }
