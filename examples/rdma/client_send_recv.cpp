#include "src/rdma_communicator.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>

static const int PORT = 7471;
static const size_t BUF_SIZE = 1024;

static void die(const char* msg){ perror(msg); exit(1); }

int main(int argc, char** argv){
    if(argc<2){ std::cerr<<"usage: client_send_recv <server_ip>\n"; return 1; }
    srand(time(nullptr));

    // ---- socket connect ----
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if(cfd<0) die("socket");
    sockaddr_in sa{}; sa.sin_family=AF_INET; sa.sin_port=htons(PORT);
    if(inet_pton(AF_INET, argv[1], &sa.sin_addr)!=1) die("inet_pton");
    if(connect(cfd,(sockaddr*)&sa,sizeof(sa))<0) die("connect");

    // Create RDMA communicator
    RDMACommunicator rdma_comm(cfd, "mlx5_1", 0);
    printf("RDMACommunicator: fd=%d, buffer_size=%ld\n", cfd, BUF_SIZE);
    
    // Create external buffer
    char* buf = (char*)aligned_alloc(4096, BUF_SIZE);
    if (!buf) die("Failed to allocate buffer");
    
    // Set buffer
    rdma_comm.set_buffer(buf, BUF_SIZE);

    // 此代码行使用 snprintf 函数将字符串 "Hello RDMA SEND via pure libibverbs." 写入到缓冲区 buf 中，
    // 最多写入 BUF_SIZE 个字符，确保不会发生缓冲区溢出。
    snprintf(buf, BUF_SIZE, "Hello RDMA SEND via pure libibverbs.");
    printf("RDMACommunicator: buf=%s\n", buf);
    
    // Exchange QP information
    WireMsg peer{}, self{};
    self.rkey = 0;  // client不提供rkey/vaddr（写对端）
    self.vaddr = 0;
    
    printf("RDMACommunicator: QP exchange\n");
    if (rdma_comm.exchange_qp_info(self, peer) != 0) die("exchange_qp_info");
    printf("RDMACommunicator: QP exchange completed\n");
    
    // Modify QP to INIT state
    if (rdma_comm.modify_qp_to_init() != 0) die("modify_qp_to_init");
    printf("RDMACommunicator: QP state=INIT\n");

    
    // Modify QP to RTR state
    if (rdma_comm.modify_qp_to_rtr(peer) != 0) die("modify_qp_to_rtr");
    printf("RDMACommunicator: QP state=RTR\n");
    
    // Modify QP to RTS state
    if (rdma_comm.modify_qp_to_rts(self) != 0) die("modify_qp_to_rts");
    printf("RDMACommunicator: QP state=RTS\n");

    // --- RDMA SEND ---
    size_t msg_len = strlen(buf) + 1;
    if (rdma_comm.send(buf, msg_len, 10) != 0) die("RDMA send failed");
    
    std::cout<<"SEND completed. bytes="<<msg_len<<"\n";

    // Prepare to receive confirmation message from server
    // Post receive buffer before waiting for data
    if (rdma_comm.post_receive(buf, BUF_SIZE) != 0) die("post_receive failed");
    
    // Wait for confirmation message from server
    int bytes_received = rdma_comm.recv(buf, BUF_SIZE);
    if (bytes_received < 0) die("RDMA recv failed");
    
    std::cout<<"Received confirmation from server: "<<buf<<" (bytes="<<bytes_received<<")"<<std::endl;

    close(cfd);
    return 0;
}