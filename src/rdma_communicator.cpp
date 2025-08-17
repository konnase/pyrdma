#include "rdma_communicator.h"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <unistd.h>
#include <cerrno>
#include <cstdio>

static void die(const char* msg) { 
    perror(msg); 
    exit(1); 
}

void RDMACommunicator::readn(int fd, void* p, size_t n) {
    uint8_t* b = (uint8_t*)p;
    size_t r = 0; 
    while (r < n) { 
        ssize_t k = ::read(fd, b + r, n - r); 
        if (k <= 0) die("read"); 
        r += k; 
    }
}

void RDMACommunicator::writen(int fd, const void* p, size_t n) {
    const uint8_t* b = (const uint8_t*)p;
    size_t s = 0; 
    while (s < n) { 
        ssize_t k = ::write(fd, b + s, n - s); 
        if (k <= 0) die("write"); 
        s += k; 
    }
}

RDMACommunicator::RDMACommunicator(int fd, size_t buffer_size) : socket_fd(fd), buf_size(buffer_size) {
    // Initialize RDMA resources
    if (init_rdma() != 0) {
        die("Failed to initialize RDMA");
    }
}

RDMACommunicator::~RDMACommunicator() {
    if (mr) ibv_dereg_mr(mr);
    if (buf) free(buf);
    if (qp) ibv_destroy_qp(qp);
    if (cq) ibv_destroy_cq(cq);
    if (pd) ibv_dealloc_pd(pd);
    if (ctx) ibv_close_device(ctx);
}

int RDMACommunicator::init_rdma() {
    // Get device list
    int num;
    ibv_device **dev_list = ibv_get_device_list(&num);
    if (!dev_list || num == 0) return -1;
    
    // Open device
    ctx = ibv_open_device(dev_list[0]);
    if (!ctx) return -1;
    
    // Free device list
    ibv_free_device_list(dev_list);
    
    // Query port
    ibv_port_attr port_attr{};
    if (ibv_query_port(ctx, IB_PORT, &port_attr)) return -1;
    
    // Allocate protection domain
    pd = ibv_alloc_pd(ctx);
    if (!pd) return -1;
    
    // Create completion queue
    cq = ibv_create_cq(ctx, CQE, nullptr, nullptr, 0);
    if (!cq) return -1;
    
    // Create queue pair
    ibv_qp_init_attr qia{};
    qia.send_cq = cq;
    qia.recv_cq = cq;
    qia.qp_type = IBV_QPT_RC;
    qia.cap.max_send_wr = 8;
    qia.cap.max_recv_wr = 8;
    qia.cap.max_send_sge = 1;
    qia.cap.max_recv_sge = 1;
    
    qp = ibv_create_qp(pd, &qia);
    if (!qp) return -1;
    
    // Allocate and register buffer
    buf = aligned_alloc(4096, buf_size);
    if (!buf) return -1;
    
    memset(buf, 0, buf_size);
    mr = ibv_reg_mr(pd, buf, buf_size,
        IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ);
    if (!mr) return -1;
    
    return 0;
}

int RDMACommunicator::modify_qp_to_init() {
    ibv_qp_attr attr{};
    attr.qp_state = IBV_QPS_INIT;
    attr.pkey_index = 0;
    attr.port_num = IB_PORT;
    attr.qp_access_flags = IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_LOCAL_WRITE;
    
    return ibv_modify_qp(qp, &attr,
        IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT | IBV_QP_ACCESS_FLAGS);
}

int RDMACommunicator::modify_qp_to_rtr(WireMsg& peer) {
    ibv_qp_attr attr{};
    attr.qp_state = IBV_QPS_RTR;
    attr.path_mtu = IBV_MTU_1024;
    attr.dest_qp_num = peer.qpn;
    attr.rq_psn = peer.psn;
    attr.max_dest_rd_atomic = 1;
    attr.min_rnr_timer = 12;
    attr.ah_attr.is_global = (peer.lid == 0); // RoCE path if no LID
    attr.ah_attr.sl = 0;
    attr.ah_attr.src_path_bits = 0;
    attr.ah_attr.port_num = IB_PORT;
    
    if (attr.ah_attr.is_global) {
        memcpy(&attr.ah_attr.grh.dgid, peer.gid, 16);
        attr.ah_attr.grh.hop_limit = 1;
        attr.ah_attr.grh.sgid_index = GID_INDEX;
        attr.ah_attr.dlid = 0;
    } else {
        attr.ah_attr.dlid = peer.lid;
    }
    
    return ibv_modify_qp(qp, &attr,
        IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU |
        IBV_QP_DEST_QPN | IBV_QP_RQ_PSN |
        IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER);
}

int RDMACommunicator::modify_qp_to_rts(WireMsg& self) {
    ibv_qp_attr attr{};
    attr.qp_state = IBV_QPS_RTS;
    attr.timeout = 14;
    attr.retry_cnt = 7;
    attr.rnr_retry = 7;
    attr.sq_psn = self.psn;
    attr.max_rd_atomic = 1;
    
    return ibv_modify_qp(qp, &attr,
        IBV_QP_STATE | IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT |
        IBV_QP_RNR_RETRY | IBV_QP_SQ_PSN | IBV_QP_MAX_QP_RD_ATOMIC);
}

int RDMACommunicator::exchange_qp_info(WireMsg& self, WireMsg& peer) {
    // Query local GID
    ibv_gid gid{};
    if (ibv_query_gid(ctx, IB_PORT, GID_INDEX, &gid)) return -1;
    
    // Fill self information
    self.qpn = qp->qp_num;
    self.psn = rand() & 0xffffff;
    
    // Query port attributes to get LID
    ibv_port_attr port_attr{};
    if (ibv_query_port(ctx, IB_PORT, &port_attr)) return -1;
    self.lid = port_attr.lid;
    
    memcpy(self.gid, &gid, 16);
    self.rkey = mr->rkey;
    self.vaddr = (uint64_t)(uintptr_t)buf;
    
    // Exchange information
    writen(socket_fd, &self, sizeof(self));
    readn(socket_fd, &peer, sizeof(peer));
    
    return 0;
}

int RDMACommunicator::send(const void* buf, size_t len) {
    writen(socket_fd, buf, len);
    return 0;
}

int RDMACommunicator::recv(void* buf, size_t len) {
    readn(socket_fd, buf, len);
    return 0;
}

int RDMACommunicator::write(const void* local_buf, size_t len, uint64_t remote_addr, uint32_t rkey) {
    ibv_sge sge{};
    sge.addr = (uintptr_t)local_buf;
    sge.length = len;
    sge.lkey = mr->lkey;
    
    ibv_send_wr wr{};
    wr.wr_id = 1;
    wr.opcode = IBV_WR_RDMA_WRITE;
    wr.sg_list = &sge;
    wr.num_sge = 1;
    wr.send_flags = IBV_SEND_SIGNALED;
    wr.wr.rdma.remote_addr = remote_addr;
    wr.wr.rdma.rkey = rkey;
    
    ibv_send_wr* bad = nullptr;
    if (ibv_post_send(qp, &wr, &bad)) return -1;
    
    // Poll completion
    ibv_wc wc{};
    int np;
    do { 
        np = ibv_poll_cq(cq, 1, &wc); 
    } while (np == 0);
    
    if (np < 0 || wc.status != IBV_WC_SUCCESS) {
        return -1;
    }
    
    return 0;
}

int RDMACommunicator::read(void* local_buf, size_t len, uint64_t remote_addr, uint32_t rkey) {
    ibv_sge sge{};
    sge.addr = (uintptr_t)local_buf;
    sge.length = len;
    sge.lkey = mr->lkey;
    
    ibv_send_wr wr{};
    wr.wr_id = 1;
    wr.opcode = IBV_WR_RDMA_READ;
    wr.sg_list = &sge;
    wr.num_sge = 1;
    wr.send_flags = IBV_SEND_SIGNALED;
    wr.wr.rdma.remote_addr = remote_addr;
    wr.wr.rdma.rkey = rkey;
    
    ibv_send_wr* bad = nullptr;
    if (ibv_post_send(qp, &wr, &bad)) return -1;
    
    // Poll completion
    ibv_wc wc{};
    int np;
    do { 
        np = ibv_poll_cq(cq, 1, &wc); 
    } while (np == 0);
    
    if (np < 0 || wc.status != IBV_WC_SUCCESS) {
        return -1;
    }
    
    return 0;
}