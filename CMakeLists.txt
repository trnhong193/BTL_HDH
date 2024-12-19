# Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(FileTransferSystem)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define the include directories
include_directories(include)

# Define the source files for the client and server
set(CLIENT_SOURCES
    client/client.cpp
    client/file_transfer.cpp
)

set(SERVER_SOURCES
    server/server.cpp
    server/file_manager.cpp
)

# Add the executable for the client
add_executable(client ${CLIENT_SOURCES})

# Add the executable for the server
add_executable(server ${SERVER_SOURCES})

# Link OpenSSL libraries for checksum_utils.h (required for MD5)
find_package(OpenSSL REQUIRED)
target_link_libraries(client OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(server OpenSSL::SSL OpenSSL::Crypto)

# Link pthread for server
target_link_libraries(server pthread)
