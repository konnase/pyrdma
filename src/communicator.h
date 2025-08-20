#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include <cstddef>
#include <cstdint>

// Base communicator class
class Communicator {
public:
    virtual ~Communicator() = default;
    
    // Pure virtual functions for send/recv operations
    virtual int send(const void* buf, size_t len, size_t offset = 0) = 0;
    virtual int recv(void* buf, size_t len, size_t offset = 0) = 0;
    
    // Pure virtual functions for RDMA operations
    virtual int write(const void* local_buf, size_t len, uint64_t remote_addr, uint32_t rkey, size_t offset = 0) = 0;
    virtual int read(void* local_buf, size_t len, uint64_t remote_addr, uint32_t rkey, size_t offset = 0) = 0;
};

#endif // COMMUNICATOR_H