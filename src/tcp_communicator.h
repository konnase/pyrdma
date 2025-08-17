#ifndef TCP_COMMUNICATOR_H
#define TCP_COMMUNICATOR_H

#include "communicator.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdint>

class TCPCommunicator : public Communicator {
private:
    int socket_fd;
    
    // Helper functions for socket operations
    static void readn(int fd, void* p, size_t n);
    static void writen(int fd, const void* p, size_t n);
    
public:
    TCPCommunicator(int fd) : socket_fd(fd) {}
    
    // Implement send/recv operations
    int send(const void* buf, size_t len) override;
    int recv(void* buf, size_t len) override;
    
    // RDMA operations are not supported in TCP
    int write(const void* /*local_buf*/, size_t /*len*/, uint64_t /*remote_addr*/, uint32_t /*rkey*/) override {
        // Not supported
        return -1;
    }
    
    int read(void* /*local_buf*/, size_t /*len*/, uint64_t /*remote_addr*/, uint32_t /*rkey*/) override {
        // Not supported
        return -1;
    }

    int close() override;
    int get_fd() override;
};

#endif // TCP_COMMUNICATOR_H