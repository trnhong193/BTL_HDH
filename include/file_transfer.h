#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

// Hàm hiển thị danh sách file từ server
void list_files(int sock);

// Hàm upload file lên server
void upload_file(int sock);

// Hàm download file từ server
void download_file(int sock);

#endif // FILE_TRANSFER_H
