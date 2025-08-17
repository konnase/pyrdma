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
static const size_t BUF_SIZE = 4096;

static void die(const char* msg){ perror(msg); exit(1); }

int main(int argc, char** argv){
    if(argc<2){ std::cerr<<"usage: client <server_ip>\n"; return 1; }
    srand(time(nullptr));

    // ---- socket connect ----
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if(cfd<0) die("socket");
    sockaddr_in sa{}; sa.sin_family=AF_INET; sa.sin_port=htons(PORT);
    if(inet_pton(AF_INET, argv[1], &sa.sin_addr)!=1) die("inet_pton");
    if(connect(cfd,(sockaddr*)&sa,sizeof(sa))<0) die("connect");

    // Create RDMA communicator
    RDMACommunicator rdma_comm(cfd, BUF_SIZE);
    
    // Get buffer and write message to it
    char* buf = (char*)rdma_comm.get_buffer();
    snprintf(buf, BUF_SIZE, "Hello RDMA WRITE via pure libibverbs.");
    
    // Exchange QP information
    WireMsg peer{}, self{};
    self.rkey = 0;  // client不提供rkey/vaddr（写对端）
    self.vaddr = 0;
    
    if (rdma_comm.exchange_qp_info(self, peer) != 0) die("exchange_qp_info");
    
    // Modify QP to INIT state
    if (rdma_comm.modify_qp_to_init() != 0) die("modify_qp_to_init");
    
    // Modify QP to RTR state
    if (rdma_comm.modify_qp_to_rtr(peer) != 0) die("modify_qp_to_rtr");
    
    // Modify QP to RTS state
    if (rdma_comm.modify_qp_to_rts(self) != 0) die("modify_qp_to_rts");

    // --- RDMA WRITE ---
    size_t msg_len = strlen(buf) + 1;
    if (rdma_comm.write(buf, msg_len, peer.vaddr, peer.rkey) != 0) die("RDMA write failed");
    
    std::cout<<"WRITE completed. bytes="<<msg_len<<"\n";

    close(cfd);
    return 0;
}
