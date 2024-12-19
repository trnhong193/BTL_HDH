#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

// Hàm liệt kê các file và thư mục trong thư mục hiện tại
void list_files(int sock);

// Hàm upload file từ client lên server
void upload_file(int sock, const char* filename);

// Hàm download file từ server gửi tới client
void download_file(int sock, const char* filename);

#endif // FILE_MANAGER_H
