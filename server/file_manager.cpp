#include <iostream>
#include <cstring>
#include <fstream>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <filesystem>
#include <cstdio>  // For popen and pclose
#include <cstring> // For strlen and strcpy
#include <cstdlib> // For malloc and free
#include <fstream>

#define MAX_BUFFER_SIZE 1000

std::string executeCommand(const char *command)
{
    // Open a pipe to the command
    FILE *pipe = popen(command, "r");
    if (!pipe)
    {
        std::cerr << "Failed to open pipe" << std::endl;
        return nullptr;
    }

    // Allocate an initial buffer to store the output
    size_t bufferSize = 1048576;
    char *result = (char *)malloc(bufferSize);
    if (!result)
    {
        std::cerr << "Memory allocation failed" << std::endl;
        pclose(pipe);
        return nullptr;
    }
    result[0] = '\0'; // Initialize the buffer as an empty string

    size_t totalSize = 0;

    // Read the output of the command
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe))
    {
        size_t bufferLen = strlen(buffer);
        if (totalSize + bufferLen + 1 > bufferSize)
        {
            // Resize the result buffer if needed
            bufferSize *= 2;
            char *temp = (char *)realloc(result, bufferSize);
            if (!temp)
            {
                std::cerr << "Memory reallocation failed" << std::endl;
                free(result);
                pclose(pipe);
                return nullptr;
            }
            result = temp;
        }

        // Append the buffer to the result
        strcpy(result + totalSize, buffer);
        totalSize += bufferLen;
    }

    pclose(pipe);
    std::string cppstr(result);
    return cppstr;
}

// Hàm đệ quy liệt kê các file và thư mục trong thư mục hiện tại
void list_files_recursive(const std::string &dir_path, int sock)
{
    DIR *dir = opendir(dir_path.c_str());
    if (dir == nullptr)
    {
        std::string msg = "Error opening directory: " + dir_path + "\n";
        send(sock, msg.c_str(), msg.size(), 0);
        return;
    }

    struct dirent *entry;
    struct stat entry_stat;

    while ((entry = readdir(dir)) != nullptr)
    {
        std::string entry_name = entry->d_name;

        // Bỏ qua thư mục "." và ".."
        if (entry_name == "." || entry_name == "..")
        {
            continue;
        }

        std::string full_path = dir_path + "/" + entry_name;

        // Kiểm tra xem entry có phải là thư mục không
        if (stat(full_path.c_str(), &entry_stat) == 0)
        {
            if (S_ISDIR(entry_stat.st_mode))
            {
                std::string msg = "DIR: " + full_path + "\n";
                send(sock, msg.c_str(), msg.size(), 0);
                list_files_recursive(full_path, sock);
            }
            else
            {
                std::string msg = "FILE: " + full_path + "\n";
                send(sock, msg.c_str(), msg.size(), 0);
            }
        }
    }

    closedir(dir);
}

// Hàm xử lý yêu cầu liệt kê file
void list_files(int sock)
{
    std::string root_dir = "/home/trnhong193/Documents/BTL_HDH_20241/uploads";
    // list_files_recursive(root_dir, sock);

    std::string cmd = "tree " + root_dir;
    std::string output = executeCommand(cmd.c_str());
    std::cout << output << std::endl;

    const std::size_t chunkSize = 1000;

    // Loop through the string in increments of chunkSize
    for (std::size_t i = 0; i < output.size(); i += chunkSize)
    {
        // Determine the end index for the current chunk
        std::size_t end = std::min(i + chunkSize, output.size());

        // Extract the substring
        std::string chunk = output.substr(i, end - i);

        int bytes_num = chunk.size();

        send(sock, chunk.c_str(), bytes_num, 0);
    }
}

// Hàm upload file từ client
void upload_file(int sock, const char *command)
{
    char buffer[MAX_BUFFER_SIZE];

    std::string filename = std::string(command).substr(7);
    std::string save_path = "/home/trnhong193/Documents/BTL_HDH_20241/uploads/" + filename;

    std::ofstream outfile(save_path, std::ios::binary);
    if (!outfile)
    {
        perror("Error: Cannot save file on server.");
        return;
    }

    int bytes_received;
    while ((bytes_received = recv(sock, buffer, MAX_BUFFER_SIZE, 0)) > 0)
    {
        if (bytes_received == 3 && strncmp(buffer, "EOF", 3) == 0)
        {
            break;
        }
        outfile.write(buffer, bytes_received);
    }

    if (outfile.good())
    {
        std::cout << "File received successfully: " << save_path << "\n";
    }
    else
    {
        std::cerr << "Error: Failed to write data to file.\n";
    }

    outfile.close();
}

// Hàm download file cho client
void download_file(int sock, const char *command)
{
    char buffer[MAX_BUFFER_SIZE];

    std::string filename = std::string(command).substr(9);
    std::string file_path = "/home/trnhong193/Documents/BTL_HDH_20241/uploads/" + filename;

    std::ifstream infile(file_path, std::ios::binary);
    if (!infile)
    {
        std::string msg = "File not found.\n";
        send(sock, msg.c_str(), msg.size(), 0);
        return;
    }
    std::cout << "start." << std::endl;

    while (infile.read(buffer, MAX_BUFFER_SIZE))
    {
        send(sock, buffer, infile.gcount(), 0);
        std::cout << "send." << std::endl;
    }
    if (infile.gcount() > 0)
    {
        send(sock, buffer, infile.gcount(), 0);
    }

    send(sock, "\x1A", 1, 0);
    infile.close();
    std::cout << "done." << std::endl;
}
