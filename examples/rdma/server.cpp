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

static void die(const char* msg){ perror(msg); exit(1); }

int main() {
    srand(time(nullptr));

    // ---- socket listen ----
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd<0) die("socket");
    int on=1; setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
    sockaddr_in sa{}; sa.sin_family=AF_INET; sa.sin_port=htons(PORT); sa.sin_addr.s_addr=INADDR_ANY;
    if(bind(lfd,(sockaddr*)&sa,sizeof(sa))<0) die("bind");
    if(listen(lfd,1)<0) die("listen");
    std::cout<<"Server listening "<<PORT<<" ...\n";
    int cfd = accept(lfd,nullptr,nullptr); if(cfd<0) die("accept");

    // Create RDMA communicator
    RDMACommunicator rdma_comm(cfd);
    
    // Exchange QP information
    WireMsg peer{}, self{};
    if (rdma_comm.exchange_qp_info(self, peer) != 0) die("exchange_qp_info");
    
    // Modify QP to INIT state
    if (rdma_comm.modify_qp_to_init() != 0) die("modify_qp_to_init");
    
    // Modify QP to RTR state
    if (rdma_comm.modify_qp_to_rtr(peer) != 0) die("modify_qp_to_rtr");
    
    // Modify QP to RTS state
    if (rdma_comm.modify_qp_to_rts(self) != 0) die("modify_qp_to_rts");

    std::cout<<"Server ready. Waiting client RDMA WRITE...\n";
    // 简单等待对端写入，然后打印缓冲区
    sleep(2);
    std::cout<<"Server buf: "<< (char*)rdma_comm.get_buffer() << std::endl;

    close(cfd); close(lfd);
    return 0;
}
