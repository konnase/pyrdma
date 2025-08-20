#include "tcp_communicator.h"
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <iostream>

static void die(const char* msg) { 
    perror(msg); 
    exit(1); 
}

int TCPCommunicator::send(const void* buf, size_t len, size_t offset) {
    const char* p = (const char*)buf + offset;
    ssize_t ret = ::send(socket_fd, p, len, 0);
    std::cout << "send " << ret << " bytes" << std::endl;
    if (ret < 0) {
        die("send");
    }
    return ret;
}

int TCPCommunicator::recv(void* buf, size_t len, size_t offset) {
    char* p = (char*)buf + offset;
    ssize_t ret = ::recv(socket_fd, p, len, 0);
    std::cout << "recv " << ret << " bytes" << std::endl;
    if (ret < 0) {
        die("recv");
    }
    return ret;
}

int TCPCommunicator::get_fd() {
    return socket_fd;
}