#ifndef RDMA_COMMUNICATOR_H
#define RDMA_COMMUNICATOR_H

#include "communicator.h"
#include <infiniband/verbs.h>
#include <cstdint>

struct WireMsg {
    uint32_t qpn;
    uint32_t psn;
    uint16_t lid;
    uint8_t  gid[16];
    uint32_t rkey;
    uint64_t vaddr;
};

class RDMACommunicator : public Communicator {
private:
    int socket_fd;
    char* device_name;
    int gid_index;
    ibv_context* ctx;
    ibv_pd* pd;
    ibv_cq* cq;
    ibv_qp* qp;
    ibv_mr* mr;
    void* buf;
    size_t buf_size;
    WireMsg peer_info;  // Store remote QP information
    
    // RDMA connection parameters
    static const int IB_PORT = 1;
    static const int DEFAULT_GID_INDEX = 0;
    static const int CQE = 16;
    
    // Helper functions
    static void readn(int fd, void* p, size_t n);
    static void writen(int fd, const void* p, size_t n);
    int init_rdma();
    
public:  // Make these methods accessible from main
    int exchange_qp_info(WireMsg& self, WireMsg& peer);
    int modify_qp_to_init();
    int modify_qp_to_rtr(WireMsg& peer);
    int modify_qp_to_rts(WireMsg& self);
    
private:  // Return to private for other members
    
public:
    RDMACommunicator(int fd, char* device_name, int gid_index = 0, size_t buffer_size = 4096);
    ~RDMACommunicator();
    
    // Post receive work request for RDMA RECV operation
    int post_receive(void* buf, size_t len);
    
    // Implement send/recv operations using RDMA SEND/RECV
    int send(const void* buf, size_t len) override;
    int recv(void* buf, size_t len) override;
    
    // Implement RDMA operations
    int write(const void* local_buf, size_t len, uint64_t remote_addr, uint32_t rkey) override;
    int read(void* local_buf, size_t len, uint64_t remote_addr, uint32_t rkey) override;
    
    // Getters for buffer information
    void* get_buffer() { return buf; }
    uint32_t get_rkey() { return mr->rkey; }
    uint64_t get_buffer_address() { return (uint64_t)(uintptr_t)buf; }
};

#endif // RDMA_COMMUNICATOR_H