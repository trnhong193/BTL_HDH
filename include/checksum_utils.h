// checksum_utils.h
#ifndef CHECKSUM_UTILS_H
#define CHECKSUM_UTILS_H

#include <string>
#include <openssl/md5.h>
#include <fstream>

std::string calculate_checksum(const std::string& filename) {
    unsigned char c[MD5_DIGEST_LENGTH];
    MD5_CTX mdContext;
    FILE* file = fopen(filename.c_str(), "rb");
    if (file == NULL) return "";

    MD5_Init(&mdContext);
    unsigned char buffer[1024];
    int bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), file)) != 0) {
        MD5_Update(&mdContext, buffer, bytes);
    }

    MD5_Final(c, &mdContext);
    fclose(file);

    std::string checksum = "";
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        char buf[3];
        sprintf(buf, "%02x", c[i]);
        checksum += buf;
    }
    return checksum;
}

#endif // CHECKSUM_UTILS_H
